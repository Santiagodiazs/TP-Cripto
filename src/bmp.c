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

/* ---- Reverse rows in-place (for bottom-up ↔ top-down conversion) ---- */

static void reverse_rows(uint8_t *pixels, uint32_t width, uint32_t height) {
    uint8_t *tmp = malloc(width);
    if (!tmp) {
        fprintf(stderr, "bmp: out of memory in reverse_rows\n");
        return;
    }
    uint32_t row;
    for (row = 0; row < height / 2; row++) {
        uint8_t *top = pixels + row * width;
        uint8_t *bot = pixels + (height - 1 - row) * width;
        memcpy(tmp, top, width);
        memcpy(top, bot, width);
        memcpy(bot, tmp, width);
    }
    free(tmp);
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
    int      bottom_up    = (height_raw > 0); /* standard BMP = bottom-up */

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

    /* Convert bottom-up to top-down internally */
    if (bottom_up) {
        reverse_rows(pixels, width, height);
    }

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

    /* Determine if we need to write bottom-up.
     * We check the height field in the stored header: if it's positive (or was
     * positive originally) the file format expects bottom-up rows. */
    int32_t height_raw = (int32_t)read_u32le(img->header + 22);
    int bottom_up = (height_raw > 0);

    if (bottom_up) {
        /* Write rows in reverse order (bottom row first) */
        uint32_t row;
        for (row = img->height; row > 0; row--) {
            const uint8_t *rowptr = img->pixels + (row - 1) * img->width;
            if (fwrite(rowptr, 1, img->width, f) != img->width) {
                fprintf(stderr, "bmp_save: error escribiendo fila %u en '%s'.\n", row - 1, path);
                fclose(f);
                return -1;
            }
        }
    } else {
        /* Top-down: write rows in order */
        if (fwrite(img->pixels, 1, img->pixel_count, f) != img->pixel_count) {
            fprintf(stderr, "bmp_save: error escribiendo pixeles en '%s'.\n", path);
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
