/*
 * test_bmp.c — Round-trip test for the BMP module.
 *
 * Usage:  ./tests/test_bmp <input.bmp> <output.bmp>
 *
 * 1. Loads input BMP (must be 8 bpp).
 * 2. Prints dimensions and reserved header bytes.
 * 3. Sets seed = 0xABCD and shadow_num = 3 in the header.
 * 4. Saves to output BMP.
 * 5. Reloads the output and verifies seed/shadow_num were preserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/bmp.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <entrada.bmp> <salida.bmp>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *in_path  = argv[1];
    const char *out_path = argv[2];
    int ok = 1;

    /* --- Load --- */
    BMPImage img;
    memset(&img, 0, sizeof(img));
    if (bmp_load(in_path, &img) != 0) {
        fprintf(stderr, "FAIL: no se pudo cargar '%s'\n", in_path);
        return EXIT_FAILURE;
    }

    printf("=== Imagen cargada: '%s' ===\n", in_path);
    printf("  Ancho      : %u px\n",  img.width);
    printf("  Alto       : %u px\n",  img.height);
    printf("  BPP        : %u\n",     img.bits_per_pixel);
    printf("  Pixel off  : %u\n",     img.pixel_offset);
    printf("  Pixel count: %u\n",     img.pixel_count);
    printf("  Seed orig  : 0x%04X\n", bmp_get_seed(&img));
    printf("  Shadow orig: %u\n",     bmp_get_shadow_num(&img));
    printf("  Primer px  : %u\n",     img.pixels[0]);
    printf("  Ultimo px  : %u\n",     img.pixels[img.pixel_count - 1]);

    /* --- Set reserved bytes --- */
    bmp_set_seed(&img, 0xABCD);
    bmp_set_shadow_num(&img, 3);

    /* --- Save --- */
    if (bmp_save(out_path, &img) != 0) {
        fprintf(stderr, "FAIL: no se pudo guardar '%s'\n", out_path);
        bmp_free(&img);
        return EXIT_FAILURE;
    }
    printf("\nGuardado en '%s'\n", out_path);

    /* --- Reload and verify --- */
    BMPImage img2;
    memset(&img2, 0, sizeof(img2));
    if (bmp_load(out_path, &img2) != 0) {
        fprintf(stderr, "FAIL: no se pudo recargar '%s'\n", out_path);
        bmp_free(&img);
        return EXIT_FAILURE;
    }

    printf("\n=== Verificacion de ronda (reload) ===\n");

    /* Dimensions */
    if (img2.width != img.width || img2.height != img.height) {
        printf("FAIL: dimensiones difieren (%ux%u vs %ux%u)\n",
               img2.width, img2.height, img.width, img.height);
        ok = 0;
    } else {
        printf("  Dimensiones: OK (%ux%u)\n", img.width, img.height);
    }

    /* Pixel count */
    if (img2.pixel_count != img.pixel_count) {
        printf("FAIL: pixel_count difiere\n");
        ok = 0;
    } else {
        printf("  pixel_count: OK\n");
    }

    /* Reserved bytes */
    if (bmp_get_seed(&img2) != 0xABCD) {
        printf("FAIL: seed = 0x%04X (esperado 0xABCD)\n", bmp_get_seed(&img2));
        ok = 0;
    } else {
        printf("  Seed       : OK (0xABCD)\n");
    }

    if (bmp_get_shadow_num(&img2) != 3) {
        printf("FAIL: shadow_num = %u (esperado 3)\n", bmp_get_shadow_num(&img2));
        ok = 0;
    } else {
        printf("  Shadow num : OK (3)\n");
    }

    /* Pixel data */
    if (memcmp(img.pixels, img2.pixels, img.pixel_count) != 0) {
        /* Find first difference */
        uint32_t di;
        for (di = 0; di < img.pixel_count; di++) {
            if (img.pixels[di] != img2.pixels[di]) {
                printf("FAIL: pixeles difieren en indice %u: orig=%u reloaded=%u\n",
                       di, img.pixels[di], img2.pixels[di]);
                break;
            }
        }
        ok = 0;
    } else {
        printf("  Pixeles    : OK (identicos)\n");
    }

    bmp_free(&img);
    bmp_free(&img2);

    printf("\nResultado: %s\n", ok ? "PASS" : "FAIL");
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
