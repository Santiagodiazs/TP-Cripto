#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
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
    printf("[TODO] Recuperacion: secret='%s', k=%d, dir='%s'\n",
           args->secret, args->k, args->dir);
    return 0;
}
