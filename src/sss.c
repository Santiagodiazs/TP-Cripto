#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <assert.h>
#include "sss.h"
#include "bmp.h"
#include "math_gf257.h"
#include "permutation.h"

/* ================================================================
 *  Helpers internos
 * ================================================================ */

/* Verifica si un nombre de archivo termina en .bmp (case-insensitive) */
static int ends_with_bmp(const char *name) {
    size_t len = strlen(name);
    if (len < 4) return 0;
    const char *ext = name + len - 4;
    return (ext[0] == '.' &&
            (ext[1] == 'b' || ext[1] == 'B') &&
            (ext[2] == 'm' || ext[2] == 'M') &&
            (ext[3] == 'p' || ext[3] == 'P'));
}

/* Comparador de strings para qsort */
static int cmp_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

/**
 * Escanea un directorio buscando archivos .bmp, devuelve un arreglo
 * de rutas completas ordenado alfabeticamente.
 * Retorna la cantidad de archivos (>= 0) o -1 en caso de error.
 * El caller debe liberar cada path y el arreglo.
 */
static int scan_bmp_files(const char *dir, char ***out_paths) {
    DIR *d = opendir(dir);
    if (!d) {
        fprintf(stderr, "sss: no se puede abrir el directorio '%s'.\n", dir);
        return -1;
    }

    /* Primera pasada: contar archivos .bmp */
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (ends_with_bmp(entry->d_name)) count++;
    }

    if (count == 0) {
        closedir(d);
        *out_paths = NULL;
        return 0;
    }

    /* Reservar arreglo */
    char **paths = malloc((size_t)count * sizeof(char *));
    if (!paths) { closedir(d); return -1; }

    /* Segunda pasada: construir rutas completas */
    rewinddir(d);
    size_t dir_len = strlen(dir);
    int needs_sep = (dir_len > 0 && dir[dir_len - 1] != '/');
    int idx = 0;

    while ((entry = readdir(d)) != NULL) {
        if (!ends_with_bmp(entry->d_name)) continue;
        size_t name_len = strlen(entry->d_name);
        size_t path_len = dir_len + (size_t)(needs_sep ? 1 : 0) + name_len + 1;
        paths[idx] = malloc(path_len);
        if (!paths[idx]) {
            for (int j = 0; j < idx; j++) free(paths[j]);
            free(paths);
            closedir(d);
            return -1;
        }
        snprintf(paths[idx], path_len, "%s%s%s",
                 dir, needs_sep ? "/" : "", entry->d_name);
        idx++;
    }
    closedir(d);

    /* Ordenar alfabeticamente */
    qsort(paths, (size_t)count, sizeof(char *), cmp_strings);

    *out_paths = paths;
    return count;
}

/**
 * Evalua P(x) = a_0 + a_1*x + ... + a_{k-1}*x^{k-1}  en GF(257)
 * usando el metodo de Horner.
 */
static uint16_t poly_eval(const uint16_t *coeffs, int k, uint16_t x) {
    uint16_t result = coeffs[k - 1];
    for (int i = k - 2; i >= 0; i--) {
        result = gf257_add(gf257_mul(result, x), coeffs[i]);
    }
    return result;
}

/**
 * Determina cuantos bits LSB usar por byte de portadora segun k.
 *   k in [2,3]   -> 4 bits  (2 bytes de portadora por byte de sombra)
 *   k in [4,7]   -> 2 bits  (4 bytes de portadora por byte de sombra)
 *   k in [8,10]  -> 1 bit   (8 bytes de portadora por byte de sombra)
 */
static int get_lsb_count(int k) {
    if (k <= 3) return 4;
    if (k <= 7) return 2;
    return 1;
}

/**
 * Embebe bytes de sombra en los LSBs de los pixeles de la portadora.
 * Orden MSB-first: el bit mas significativo del byte de sombra se
 * almacena en el primer pixel de portadora consumido.
 *
 * lsb_count: 1, 2 o 4 bits reemplazados por byte de portadora.
 */
static void lsb_embed(uint8_t *carrier_pixels, const uint8_t *shadow,
                       uint32_t shadow_len, int lsb_count) {
    uint8_t mask  = (uint8_t)((1 << lsb_count) - 1);
    uint8_t clear = (uint8_t)~mask;
    int chunks_per_byte = 8 / lsb_count;
    uint32_t ci = 0;

    for (uint32_t si = 0; si < shadow_len; si++) {
        for (int g = 0; g < chunks_per_byte; g++) {
            int shift = 8 - lsb_count * (g + 1);
            uint8_t bits = (uint8_t)((shadow[si] >> shift) & mask);
            carrier_pixels[ci] = (carrier_pixels[ci] & clear) | bits;
            ci++;
        }
    }
}

/**
 * Extrae bytes de sombra desde los LSBs de los pixeles de la portadora.
 * Orden MSB-first.
 */
static void lsb_extract(const uint8_t *carrier_pixels, uint8_t *shadow,
                        uint32_t shadow_len, int lsb_count) {
    uint8_t mask = (uint8_t)((1 << lsb_count) - 1);
    int chunks_per_byte = 8 / lsb_count;
    uint32_t ci = 0;

    for (uint32_t si = 0; si < shadow_len; si++) {
        uint8_t val = 0;
        for (int g = 0; g < chunks_per_byte; g++) {
            int shift = 8 - lsb_count * (g + 1);
            uint8_t bits = carrier_pixels[ci] & mask;
            val |= (uint8_t)(bits << shift);
            ci++;
        }
        shadow[si] = val;
    }
}

/* ================================================================
 *  sss_distribute
 * ================================================================ */

int sss_distribute(const Args *args) {
    int ret = -1;
    BMPImage secret;
    memset(&secret, 0, sizeof(secret));
    char **carrier_paths = NULL;
    int carrier_count    = 0;
    uint8_t **shadows    = NULL;
    uint8_t *padded      = NULL;
    int n = 0;

    /* 1. Cargar imagen secreta */
    if (bmp_load(args->secret, &secret) != 0) return -1;
    printf("[SSS] Imagen secreta cargada: %ux%u (%u pixeles)\n",
           secret.width, secret.height, secret.pixel_count);

    int k = args->k;

    /* 2. Escanear directorio de portadoras */
    carrier_count = scan_bmp_files(args->dir, &carrier_paths);
    if (carrier_count < 0) goto cleanup;
    if (carrier_count == 0) {
        fprintf(stderr, "sss: no se encontraron archivos BMP en '%s'.\n", args->dir);
        goto cleanup;
    }

    /* 3. Determinar n */
    n = args->n;
    if (n == 0) n = carrier_count;
    if (n < k) {
        fprintf(stderr, "sss: n=%d < k=%d. Se necesitan al menos k portadoras.\n", n, k);
        goto cleanup;
    }
    if (carrier_count < n) {
        fprintf(stderr,
                "sss: se necesitan %d portadoras pero solo se encontraron %d en '%s'.\n",
                n, carrier_count, args->dir);
        goto cleanup;
    }
    printf("[SSS] Usando %d portadoras (k=%d, n=%d)\n", n, k, n);

    /* 4. Generar semilla aleatoria y aplicar permutacion XOR */
    srand((unsigned)time(NULL));
    uint16_t seed = (uint16_t)(rand() & 0xFFFF);
    printf("[SSS] Semilla generada: 0x%04X\n", seed);
    permutation_xor_mask(secret.pixels, secret.pixel_count, seed);

    /* 5. Calcular tamanio de cada sombra */
    uint32_t M = secret.pixel_count;
    uint32_t shadow_len = (M + (uint32_t)k - 1) / (uint32_t)k;

    /* 6. Reservar arreglos de sombras (n arreglos de shadow_len bytes) */
    shadows = calloc((size_t)n, sizeof(uint8_t *));
    if (!shadows) goto cleanup;
    for (int i = 0; i < n; i++) {
        shadows[i] = calloc(shadow_len, 1);
        if (!shadows[i]) goto cleanup;
    }

    /* 7. Paddear pixeles del secreto para que sean divisibles por k */
    {
        uint32_t padded_len = shadow_len * (uint32_t)k;
        padded = calloc(padded_len, 1);
        if (!padded) goto cleanup;
        memcpy(padded, secret.pixels, M);
        /* Los bytes restantes quedan en 0 (padding) */
    }

    /* 8. Generar polinomios y evaluar sombras */
    for (uint32_t block = 0; block < shadow_len; block++) {
        uint16_t coeffs[10]; /* k <= 10 */
        for (int c = 0; c < k; c++) {
            coeffs[c] = padded[block * (uint32_t)k + (uint32_t)c];
        }

        /* Evaluar en x = 1..n  con mitigacion del 256 */
        int valid = 0;
        while (!valid) {
            valid = 1;
            for (int j = 0; j < n; j++) {
                uint16_t val = poly_eval(coeffs, k, (uint16_t)(j + 1));
                if (val == 256) {
                    coeffs[0] = (coeffs[0] + 1) % 256;
                    valid = 0;
                    break; /* reiniciar evaluaciones para este bloque */
                }
                shadows[j][block] = (uint8_t)val;
            }
        }
    }
    printf("[SSS] %u bloques procesados (shadow_len = %u bytes)\n",
           shadow_len, shadow_len);

    free(padded);
    padded = NULL;

    /* 9. Embeber cada sombra en su portadora */
    {
        int lsb_count = get_lsb_count(k);
        uint32_t pixels_needed = shadow_len * (uint32_t)(8 / lsb_count);

        for (int i = 0; i < n; i++) {
            BMPImage carrier;
            memset(&carrier, 0, sizeof(carrier));

            if (bmp_load(carrier_paths[i], &carrier) != 0) {
                fprintf(stderr, "sss: no se pudo cargar la portadora '%s'.\n",
                        carrier_paths[i]);
                goto cleanup;
            }

            /* Validar capacidad de la portadora */
            if (carrier.pixel_count < pixels_needed) {
                fprintf(stderr,
                        "sss: portadora '%s' tiene %u pixeles, se necesitan %u.\n",
                        carrier_paths[i], carrier.pixel_count, pixels_needed);
                bmp_free(&carrier);
                goto cleanup;
            }

            /* Escribir metadatos en la cabecera */
            bmp_set_seed(&carrier, seed);
            bmp_set_shadow_num(&carrier, (uint16_t)(i + 1));

            /* Embeber sombra en los LSBs */
            lsb_embed(carrier.pixels, shadows[i], shadow_len, lsb_count);

            /* Guardar portadora modificada */
            if (bmp_save(carrier_paths[i], &carrier) != 0) {
                fprintf(stderr, "sss: no se pudo guardar '%s'.\n", carrier_paths[i]);
                bmp_free(&carrier);
                goto cleanup;
            }

            printf("[SSS] Sombra %d embebida en '%s'\n", i + 1, carrier_paths[i]);
            bmp_free(&carrier);
        }
    }

    printf("[SSS] Distribucion completada exitosamente.\n");
    ret = 0;

cleanup:
    if (padded) free(padded);
    if (shadows) {
        for (int i = 0; i < n; i++) {
            if (shadows[i]) free(shadows[i]);
        }
        free(shadows);
    }
    if (carrier_paths) {
        for (int i = 0; i < carrier_count; i++) free(carrier_paths[i]);
        free(carrier_paths);
    }
    bmp_free(&secret);
    return ret;
}

/* ================================================================
 *  sss_recover  (Fase 4 — stub)
 * ================================================================ */

int sss_recover(const Args *args) {
    int ret = -1;
    char **carrier_paths = NULL;
    int carrier_count = 0;
    BMPImage carriers[10];
    uint16_t shadow_indices[10];
    int active_count = 0;
    uint8_t **shadows = NULL;
    uint8_t *recovered_pixels = NULL;
    BMPImage secret_out;

    memset(carriers, 0, sizeof(carriers));
    memset(shadow_indices, 0, sizeof(shadow_indices));
    memset(&secret_out, 0, sizeof(secret_out));

    int k = args->k;
    const char *dir = args->dir;
    const char *out_secret_path = args->secret;

    /* 1. Escanear directorio de portadoras */
    carrier_count = scan_bmp_files(dir, &carrier_paths);
    if (carrier_count < 0) return -1;
    if (carrier_count == 0) {
        fprintf(stderr, "sss: no se encontraron archivos BMP en '%s'.\n", dir);
        goto cleanup;
    }

    /* 2. Cargar portadoras validas unicas */
    for (int i = 0; i < carrier_count && active_count < k; i++) {
        BMPImage img;
        memset(&img, 0, sizeof(img));
        if (bmp_load(carrier_paths[i], &img) != 0) {
            continue; /* Ignorar archivo no BMP u 8bpp invalido */
        }
        uint16_t shadow_num = bmp_get_shadow_num(&img);
        if (shadow_num == 0 || shadow_num > 256) {
            bmp_free(&img);
            continue; /* Ignorar portadoras sin indice valido */
        }
        /* Verificar si ya cargamos una portadora con este shadow_num */
        int duplicate = 0;
        for (int j = 0; j < active_count; j++) {
            if (shadow_indices[j] == shadow_num) {
                duplicate = 1;
                break;
            }
        }
        if (duplicate) {
            bmp_free(&img);
            continue;
        }

        /* Guardar portadora valida */
        carriers[active_count] = img;
        shadow_indices[active_count] = shadow_num;
        active_count++;
    }

    if (active_count < k) {
        fprintf(stderr, "sss: se necesitan al menos %d portadoras validas con indices de sombra unicos para recuperar, pero solo se encontraron %d.\n", k, active_count);
        goto cleanup;
    }

    /* 3. Validar consistencia de dimensiones */
    uint32_t width = carriers[0].width;
    uint32_t height = carriers[0].height;
    uint32_t M = carriers[0].pixel_count;

    for (int i = 1; i < k; i++) {
        if (carriers[i].width != width || carriers[i].height != height) {
            fprintf(stderr, "sss: error, las portadoras no tienen las mismas dimensiones (%ux%u vs %ux%u).\n",
                    width, height, carriers[i].width, carriers[i].height);
            goto cleanup;
        }
    }

    /* 4. Obtener semilla de la primera portadora */
    uint16_t seed = bmp_get_seed(&carriers[0]);
    printf("[SSS] Semilla extraida de la cabecera: 0x%04X\n", seed);

    /* 5. Calcular longitud de sombra y comprobar capacidad */
    int lsb_count = get_lsb_count(k);
    uint32_t shadow_len = (M + (uint32_t)k - 1) / (uint32_t)k;
    uint32_t pixels_needed = shadow_len * (uint32_t)(8 / lsb_count);

    if (M < pixels_needed) {
        fprintf(stderr, "sss: error, las portadoras tienen menor cantidad de pixeles que la necesaria para la sombra (%u < %u).\n", M, pixels_needed);
        goto cleanup;
    }

    /* 6. Reservar memoria y extraer las sombras */
    shadows = calloc((size_t)k, sizeof(uint8_t *));
    if (!shadows) goto cleanup;
    for (int i = 0; i < k; i++) {
        shadows[i] = malloc(shadow_len);
        if (!shadows[i]) goto cleanup;
    }

    for (int i = 0; i < k; i++) {
        lsb_extract(carriers[i].pixels, shadows[i], shadow_len, lsb_count);
    }
    printf("[SSS] Sombras extraidas exitosamente.\n");

    /* 7. Reservar buffer para pixeles recuperados */
    uint32_t padded_len = shadow_len * (uint32_t)k;
    recovered_pixels = malloc(padded_len);
    if (!recovered_pixels) goto cleanup;

    /* 8. Reconstruccion por bloques usando Lagrange Reducido */
    for (uint32_t block = 0; block < shadow_len; block++) {
        uint16_t x[10];
        uint16_t y[10];
        uint16_t coeffs[10];

        for (int i = 0; i < k; i++) {
            x[i] = shadow_indices[i];
            y[i] = shadows[i][block];
        }

        for (int j = 0; j < k; j++) {
            int num_points = k - j;
            uint16_t a_j = 0;

            for (int i = 0; i < num_points; i++) {
                uint16_t num = 1;
                uint16_t den = 1;
                for (int m = 0; m < num_points; m++) {
                    if (m == i) continue;
                    uint16_t neg_xm = (uint16_t)((257 - x[m]) % 257);
                    num = gf257_mul(num, neg_xm);
                    den = gf257_mul(den, gf257_sub(x[i], x[m]));
                }
                uint16_t term = gf257_mul(y[i], gf257_div(num, den));
                a_j = gf257_add(a_j, term);
            }

            coeffs[j] = a_j;

            /* Reducir el grado para la proxima iteracion */
            if (j < k - 1) {
                for (int i = 0; i < num_points - 1; i++) {
                    uint16_t num_val = gf257_sub(y[i], a_j);
                    y[i] = gf257_div(num_val, x[i]);
                }
            }
        }

        for (int j = 0; j < k; j++) {
            uint16_t val = coeffs[j];
            if (val >= 256) {
                val = 0;
            }
            recovered_pixels[block * (uint32_t)k + (uint32_t)j] = (uint8_t)val;
        }
    }

    /* 9. Desaplicar mascara XOR (Permutacion inversa) */
    permutation_xor_mask(recovered_pixels, M, seed);

    /* 10. Clonar encabezado y construir BMP de salida */
    secret_out.width = width;
    secret_out.height = height;
    secret_out.pixel_offset = carriers[0].pixel_offset;
    secret_out.bits_per_pixel = carriers[0].bits_per_pixel;
    secret_out.pixel_count = M;
    secret_out.header_size = carriers[0].header_size;
    memcpy(secret_out.header, carriers[0].header, carriers[0].header_size);

    /* Limpiar metadatos en la cabecera restaurando valores originales */
    bmp_set_seed(&secret_out, 0);
    bmp_set_shadow_num(&secret_out, 0);

    secret_out.pixels = malloc(M);
    if (!secret_out.pixels) goto cleanup;
    memcpy(secret_out.pixels, recovered_pixels, M);

    /* 11. Guardar la imagen */
    if (bmp_save(out_secret_path, &secret_out) != 0) {
        fprintf(stderr, "sss: no se pudo guardar el secreto en '%s'.\n", out_secret_path);
        goto cleanup;
    }

    printf("[SSS] Recuperacion completada exitosamente. Imagen guardada en '%s'.\n", out_secret_path);
    ret = 0;

cleanup:
    bmp_free(&secret_out);
    if (recovered_pixels) free(recovered_pixels);
    if (shadows) {
        for (int i = 0; i < k; i++) {
            if (shadows[i]) free(shadows[i]);
        }
        free(shadows);
    }
    for (int i = 0; i < active_count; i++) {
        bmp_free(&carriers[i]);
    }
    if (carrier_paths) {
        for (int i = 0; i < carrier_count; i++) free(carrier_paths[i]);
        free(carrier_paths);
    }
    return ret;
}
