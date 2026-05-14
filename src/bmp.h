#ifndef BMP_H
#define BMP_H

#include <stdint.h>

/* Maximum header we ever keep in memory.
 * 8bpp BMPs with a full 256-entry color table have pixel_offset = 1078
 * (54-byte file+DIB header + 256×4-byte color table). Use 2048 to be safe. */
#define BMP_MAX_HEADER 2048

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pixel_offset;  /* byte offset where pixel data starts    */
    uint16_t bits_per_pixel;
    uint8_t *pixels;        /* pixel buffer, row-major, top-down      */
    uint32_t pixel_count;   /* width * height                         */
    uint8_t  header[BMP_MAX_HEADER]; /* raw header bytes               */
    uint32_t header_size;   /* actual number of bytes in header[]     */
} BMPImage;

/**
 * Load a BMP file into *img.
 * Only 8 bpp (grayscale) images are accepted.
 * Returns 0 on success, -1 on error.
 */
int  bmp_load(const char *path, BMPImage *img);

/**
 * Save *img to a BMP file at path.
 * Returns 0 on success, -1 on error.
 */
int  bmp_save(const char *path, const BMPImage *img);

/** Free heap memory inside img (does NOT free img itself). */
void bmp_free(BMPImage *img);

/* --- Reserved header byte accessors ---
 *
 * Bytes 6-7  → permutation seed  (stored little-endian)
 * Bytes 8-9  → shadow number 1..n (stored little-endian)
 */
uint16_t bmp_get_seed(const BMPImage *img);
void     bmp_set_seed(BMPImage *img, uint16_t seed);

uint16_t bmp_get_shadow_num(const BMPImage *img);
void     bmp_set_shadow_num(BMPImage *img, uint16_t num);

#endif /* BMP_H */
