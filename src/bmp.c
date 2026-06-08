#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "bmp.h"

/* ---- Little-endian readers (portable, no alignment assumptions) ---- */

static uint16_t read_u16le(const uint8_t *buf) {
    return (uint16_t)(buf[0] | ((uint16_t)buf[1] << 8));
}

static uint32_t read_u32le(const uint8_t *buf) {
    return (uint32_t)( buf[0]
                     | ((uint32_t)buf[1] <<  8)
                     | ((uint32_t)buf[2] << 16)
                     | ((uint32_t)buf[3] << 24) );
}

static void write_u16le(uint8_t *buf, uint16_t v) {
    buf[0] = (uint8_t)(v & 0xFF);
    buf[1] = (uint8_t)((v >> 8) & 0xFF);
}



/* ================================================================
 *  bmp_load
 * ================================================================ */
int bmp_load(const char *path, BMPImage *img) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "bmp_load: no se puede abrir '%s': %s\n", path, strerror(errno));
        return -1;
    }

    /* Read the first 54 bytes (standard minimum BMP header) */
    uint8_t hdr54[54];
    if (fread(hdr54, 1, 54, f) != 54) {
        fprintf(stderr, "bmp_load: '%s' es demasiado pequeno para ser un BMP valido.\n", path);
        fclose(f);
        return -1;
    }

    /* Verify BMP magic */
    if (hdr54[0] != 'B' || hdr54[1] != 'M') {
        fprintf(stderr, "bmp_load: '%s' no es un archivo BMP (magic incorrecto).\n", path);
        fclose(f);
        return -1;
    }

    /* Read fields from the header */
    uint32_t pixel_offset = read_u32le(hdr54 + 10);
    uint16_t bpp          = read_u16le(hdr54 + 28);
    uint32_t width        = read_u32le(hdr54 + 18);
    int32_t  height_raw   = (int32_t)read_u32le(hdr54 + 22);
    uint32_t height       = (uint32_t)(height_raw < 0 ? -height_raw : height_raw);


    /* Validate */
    if (bpp != 8) {
        fprintf(stderr, "bmp_load: '%s' tiene %u bpp; solo se soportan imagenes de 8 bpp (escala de grises).\n",
                path, bpp);
        fclose(f);
        return -1;
    }
    if (width == 0 || height == 0) {
        fprintf(stderr, "bmp_load: '%s' tiene dimensiones invalidas (%ux%u).\n", path, width, height);
        fclose(f);
        return -1;
    }
    if (pixel_offset > BMP_MAX_HEADER) {
        fprintf(stderr, "bmp_load: '%s' tiene pixel_offset=%u > BMP_MAX_HEADER=%u. Aumentar BMP_MAX_HEADER.\n",
                path, pixel_offset, BMP_MAX_HEADER);
        fclose(f);
        return -1;
    }

    /* Read the full header (up to pixel_offset bytes) */
    img->header_size = pixel_offset;
    memcpy(img->header, hdr54, 54);
    if (pixel_offset > 54) {
        uint32_t extra = pixel_offset - 54;
        if (fread(img->header + 54, 1, extra, f) != extra) {
            fprintf(stderr, "bmp_load: no se pudo leer el encabezado completo de '%s'.\n", path);
            fclose(f);
            return -1;
        }
    }

    /* Seek to pixel data */
    if (fseek(f, (long)pixel_offset, SEEK_SET) != 0) {
        fprintf(stderr, "bmp_load: fseek fallo en '%s'.\n", path);
        fclose(f);
        return -1;
    }

    /* Allocate and read pixel buffer */
    uint32_t pixel_count = width * height;
    uint8_t *pixels = malloc(pixel_count);
    if (!pixels) {
        fprintf(stderr, "bmp_load: sin memoria para %u pixeles.\n", pixel_count);
        fclose(f);
        return -1;
    }

    if (fread(pixels, 1, pixel_count, f) != pixel_count) {
        fprintf(stderr, "bmp_load: no se pudieron leer todos los pixeles de '%s'.\n", path);
        free(pixels);
        fclose(f);
        return -1;
    }
    fclose(f);

    /* Row reversal is disabled to keep raw file order */

    /* Fill the struct */
    img->width        = width;
    img->height       = height;
    img->pixel_offset = pixel_offset;
    img->bits_per_pixel = bpp;
    img->pixels       = pixels;
    img->pixel_count  = pixel_count;

    /* If the image was bottom-up, mark height as positive in the stored header.
     * We keep the header bytes as-is (they will be re-reversed on save). */

#ifdef DEBUG
    printf("[BMP] Cargado '%s': %ux%u px, %u bpp, pixel_offset=%u (%s)\n",
           path, width, height, bpp, pixel_offset,
           bottom_up ? "bottom-up" : "top-down");
#endif

    return 0;
}

/* ================================================================
 *  bmp_save
 * ================================================================ */
int bmp_save(const char *path, const BMPImage *img) {
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "bmp_save: no se puede crear '%s': %s\n", path, strerror(errno));
        return -1;
    }

    /* Write header */
    if (fwrite(img->header, 1, img->header_size, f) != img->header_size) {
        fprintf(stderr, "bmp_save: error escribiendo encabezado en '%s'.\n", path);
        fclose(f);
        return -1;
    }



    /* BMP requiere que cada fila quede padeada a un múltiplo de 4 bytes.
     * El padding sólo se inserta al escribir; bmp_load sigue leyendo el
     * área de píxeles como buffer contiguo (esa es la convención sobre
     * la que opera el LSB replacement del esquema Wu/Lo, y romperla
     * destruiría la interoperabilidad con sombras de otras
     * implementaciones).
     *
     * El buffer `img->pixels` contiene los `width*height` bytes que están
     * en el área de píxeles del archivo de origen, contiguos. Cuando el
     * width no es múltiplo de 4 esa vista contigua incluye los bytes de
     * padding entremezclados cada `width` posiciones --- es decir, la
     * "stride efectiva" del buffer es `width + row_pad`. Al escribir hay
     * que recorrer el buffer en pasos de `stride` para reproducir el
     * layout BMP estándar y que cualquier visor renderice las filas
     * alineadas. */
    uint32_t row_pad = (4 - (img->width % 4)) % 4;
    uint32_t stride  = img->width + row_pad;
    static const uint8_t zero_pad[3] = {0, 0, 0};

    for (uint32_t row = 0; row < img->height; row++) {
        size_t row_start = (size_t)row * (size_t)stride;
        size_t available = (row_start < img->pixel_count)
                             ? img->pixel_count - row_start
                             : 0;
        size_t to_write  = available < img->width ? available : img->width;

        if (to_write > 0) {
            if (fwrite(img->pixels + row_start, 1, to_write, f) != to_write) {
                fprintf(stderr, "bmp_save: error escribiendo fila %u en '%s'.\n", row, path);
                fclose(f);
                return -1;
            }
        }
        /* Si la fila quedó corta por buffer insuficiente (caso de las
         * últimas filas al recuperar un secreto con width no divisible
         * por 4, donde el buffer de width*height bytes no llega a cubrir
         * todas las filas con stride padeada), rellenar con ceros. */
        if (to_write < img->width) {
            size_t missing = img->width - to_write;
            for (size_t i = 0; i < missing; i++) {
                if (fputc(0, f) == EOF) {
                    fprintf(stderr, "bmp_save: error rellenando fila %u en '%s'.\n", row, path);
                    fclose(f);
                    return -1;
                }
            }
        }
        if (row_pad > 0 &&
            fwrite(zero_pad, 1, row_pad, f) != row_pad) {
            fprintf(stderr, "bmp_save: error escribiendo padding de fila %u en '%s'.\n", row, path);
            fclose(f);
            return -1;
        }
    }

    fclose(f);

#ifdef DEBUG
    printf("[BMP] Guardado '%s': %ux%u px\n", path, img->width, img->height);
#endif

    return 0;
}

/* ================================================================
 *  bmp_free
 * ================================================================ */
void bmp_free(BMPImage *img) {
    if (img && img->pixels) {
        free(img->pixels);
        img->pixels = NULL;
    }
}

/* ================================================================
 *  Reserved header byte accessors
 *  Bytes 6-7  = seed     (little-endian uint16)
 *  Bytes 8-9  = shadow # (little-endian uint16)
 * ================================================================ */

uint16_t bmp_get_seed(const BMPImage *img) {
    return read_u16le(img->header + 6);
}

void bmp_set_seed(BMPImage *img, uint16_t seed) {
    write_u16le(img->header + 6, seed);
}

uint16_t bmp_get_shadow_num(const BMPImage *img) {
    return read_u16le(img->header + 8);
}

void bmp_set_shadow_num(BMPImage *img, uint16_t num) {
    write_u16le(img->header + 8, num);
}
