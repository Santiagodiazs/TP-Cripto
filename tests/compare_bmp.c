#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/bmp.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <original.bmp> <recuperado.bmp>\n", argv[0]);
        return EXIT_FAILURE;
    }
    BMPImage img1, img2;
    memset(&img1, 0, sizeof(img1));
    memset(&img2, 0, sizeof(img2));
    if (bmp_load(argv[1], &img1) != 0 || bmp_load(argv[2], &img2) != 0) {
        return EXIT_FAILURE;
    }
    if (img1.width != img2.width || img1.height != img2.height) {
        printf("FAIL: Dimensiones diferentes (%ux%u vs %ux%u)\n", img1.width, img1.height, img2.width, img2.height);
        bmp_free(&img1);
        bmp_free(&img2);
        return EXIT_FAILURE;
    }
    uint32_t diff_count = 0;
    uint32_t max_diff = 0;
    for (uint32_t i = 0; i < img1.pixel_count; i++) {
        if (img1.pixels[i] != img2.pixels[i]) {
            diff_count++;
            uint32_t diff = abs((int)img1.pixels[i] - (int)img2.pixels[i]);
            if (diff > max_diff) {
                max_diff = diff;
            }
        }
    }
    printf("Comparacion de pixeles:\n");
    printf("  Total pixeles  : %u\n", img1.pixel_count);
    printf("  Pixeles dif    : %u (%.4f%%)\n", diff_count, (double)diff_count * 100.0 / img1.pixel_count);
    printf("  Diferencia max : %u\n", max_diff);
    bmp_free(&img1);
    bmp_free(&img2);
    return EXIT_SUCCESS;
}
