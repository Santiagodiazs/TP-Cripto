#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/math_gf257.h"

/* ================================================================
 *  Duplicados locales de funciones estaticas de sss.c para testeo
 * ================================================================ */

/* Evaluacion polinomial con Horner en GF(257) */
static uint16_t poly_eval(const uint16_t *coeffs, int k, uint16_t x) {
    uint16_t result = coeffs[k - 1];
    for (int i = k - 2; i >= 0; i--) {
        result = gf257_add(gf257_mul(result, x), coeffs[i]);
    }
    return result;
}

/* Embeber bytes de sombra en LSBs de portadora (MSB-first) */
static void lsb_embed(uint8_t *carrier, const uint8_t *shadow,
                       uint32_t shadow_len, int lsb_count) {
    uint8_t mask  = (uint8_t)((1 << lsb_count) - 1);
    uint8_t clear = (uint8_t)~mask;
    int chunks = 8 / lsb_count;
    uint32_t ci = 0;
    for (uint32_t si = 0; si < shadow_len; si++) {
        for (int g = 0; g < chunks; g++) {
            int shift = 8 - lsb_count * (g + 1);
            uint8_t bits = (uint8_t)((shadow[si] >> shift) & mask);
            carrier[ci] = (carrier[ci] & clear) | bits;
            ci++;
        }
    }
}

/* Extraer bytes de sombra desde LSBs de portadora (MSB-first) */
static void lsb_extract(const uint8_t *carrier, uint8_t *shadow,
                          uint32_t shadow_len, int lsb_count) {
    uint8_t mask = (uint8_t)((1 << lsb_count) - 1);
    int chunks = 8 / lsb_count;
    uint32_t ci = 0;
    for (uint32_t si = 0; si < shadow_len; si++) {
        shadow[si] = 0;
        for (int g = 0; g < chunks; g++) {
            int shift = 8 - lsb_count * (g + 1);
            shadow[si] = (uint8_t)(shadow[si] | ((carrier[ci] & mask) << shift));
            ci++;
        }
    }
}

/* ================================================================
 *  Tests
 * ================================================================ */

static void test_poly_eval(void) {
    printf("--- Prueba de Evaluacion Polinomial (Horner en GF(257)) ---\n");

    /* Ejemplo del transcript: a_0=50, a_1=51, a_2=55, k=3 */
    /* P(x) = 50 + 51x + 55x^2 */
    uint16_t c1[] = {50, 51, 55};

    /* P(1) = 50 + 51 + 55 = 156 */
    assert(poly_eval(c1, 3, 1) == 156);

    /* P(2) = 50 + 102 + 220 = 372 mod 257 = 115 */
    assert(poly_eval(c1, 3, 2) == 115);

    /* P(3) = 50 + 153 + 495 = 698 mod 257 = 184 */
    assert(poly_eval(c1, 3, 3) == 184);
    printf("  Ejemplo del transcript (k=3): OK\n");

    /* P(0) = a_0  (caso base: recuperacion en x=0 da el termino independiente) */
    assert(poly_eval(c1, 3, 0) == 50);
    printf("  Evaluacion en x=0 (termino independiente): OK\n");

    /* Polinomio constante: P(x) = 42 */
    uint16_t c2[] = {42};
    assert(poly_eval(c2, 1, 0) == 42);
    assert(poly_eval(c2, 1, 5) == 42);
    assert(poly_eval(c2, 1, 200) == 42);
    printf("  Polinomio constante (k=1): OK\n");

    /* Polinomio lineal: P(x) = 0 + 1*x */
    uint16_t c3[] = {0, 1};
    assert(poly_eval(c3, 2, 0) == 0);
    assert(poly_eval(c3, 2, 100) == 100);
    printf("  Polinomio lineal (k=2): OK\n");

    /* Polinomio de grado 7 (k=8): simular un bloque de 8 pixeles */
    uint16_t c4[] = {10, 20, 30, 40, 50, 60, 70, 80};
    uint16_t val = poly_eval(c4, 8, 1);
    /* P(1) = 10+20+30+40+50+60+70+80 = 360 mod 257 = 103 */
    assert(val == 103);
    printf("  Polinomio de grado 7 (k=8): OK\n");

    printf("Pruebas de Evaluacion Polinomial: PASS\n\n");
}

static void test_lsb_roundtrip(void) {
    printf("--- Prueba de Round-Trip LSB (Embed + Extract) ---\n");

    uint8_t shadow[] = {0xA5, 0x3C, 0xFF, 0x00, 0x7E, 0x81};
    uint32_t shadow_len = 6;

    /* Test LSB1 (k >= 8): 8 bytes de portadora por byte de sombra */
    {
        uint8_t carrier[48];
        uint8_t recovered[6];
        for (int i = 0; i < 48; i++) carrier[i] = (uint8_t)(i * 7 + 13);

        lsb_embed(carrier, shadow, shadow_len, 1);
        lsb_extract(carrier, recovered, shadow_len, 1);
        assert(memcmp(shadow, recovered, shadow_len) == 0);
        printf("  LSB1 (k>=8): OK\n");
    }

    /* Test LSB2 (k = 4..7): 4 bytes de portadora por byte de sombra */
    {
        uint8_t carrier[24];
        uint8_t recovered[6];
        for (int i = 0; i < 24; i++) carrier[i] = (uint8_t)(i * 11 + 3);

        lsb_embed(carrier, shadow, shadow_len, 2);
        lsb_extract(carrier, recovered, shadow_len, 2);
        assert(memcmp(shadow, recovered, shadow_len) == 0);
        printf("  LSB2 (k=4..7): OK\n");
    }

    /* Test LSB4 (k = 2..3): 2 bytes de portadora por byte de sombra */
    {
        uint8_t carrier[12];
        uint8_t recovered[6];
        for (int i = 0; i < 12; i++) carrier[i] = (uint8_t)(i * 5 + 77);

        lsb_embed(carrier, shadow, shadow_len, 4);
        lsb_extract(carrier, recovered, shadow_len, 4);
        assert(memcmp(shadow, recovered, shadow_len) == 0);
        printf("  LSB4 (k=2..3): OK\n");
    }

    /* Test con todos los valores de byte posibles (0..255) en LSB1 */
    {
        uint8_t all_vals[256];
        uint8_t carrier[2048];
        uint8_t recovered[256];
        for (int i = 0; i < 256; i++) all_vals[i] = (uint8_t)i;
        for (int i = 0; i < 2048; i++) carrier[i] = (uint8_t)(i % 200 + 50);

        lsb_embed(carrier, all_vals, 256, 1);
        lsb_extract(carrier, recovered, 256, 1);
        assert(memcmp(all_vals, recovered, 256) == 0);
        printf("  LSB1 con todos los bytes (0..255): OK\n");
    }

    printf("Pruebas de Round-Trip LSB: PASS\n\n");
}

static void test_256_mitigation(void) {
    printf("--- Prueba de Mitigacion del 256 ---\n");

    /* Caso 1: P(x) = 200 + 56x, k=2 */
    /* P(1) = 200 + 56 = 256  → debe perturbarse */
    {
        uint16_t coeffs[] = {200, 56};
        assert(poly_eval(coeffs, 2, 1) == 256);

        /* Aplicar mitigacion: a_0 = (200 + 1) % 256 = 201 */
        coeffs[0] = (coeffs[0] + 1) % 256;
        assert(coeffs[0] == 201);

        /* P(1) = 201 + 56 = 257 mod 257 = 0 → valido */
        assert(poly_eval(coeffs, 2, 1) == 0);

        /* Verificar que todas las evaluaciones x=1..10 esten en [0,255] */
        for (int x = 1; x <= 10; x++) {
            uint16_t val = poly_eval(coeffs, 2, (uint16_t)x);
            assert(val != 256);
        }
        printf("  Caso 1 (P(1)=256): OK\n");
    }

    /* Caso 2: Simular el loop completo de mitigacion */
    {
        uint16_t coeffs[] = {200, 56};
        int n = 5, k = 2;
        uint8_t shadow_vals[5];

        int valid = 0;
        int iterations = 0;
        while (!valid) {
            valid = 1;
            for (int j = 0; j < n; j++) {
                uint16_t val = poly_eval(coeffs, k, (uint16_t)(j + 1));
                if (val == 256) {
                    coeffs[0] = (coeffs[0] + 1) % 256;
                    valid = 0;
                    iterations++;
                    break;
                }
                shadow_vals[j] = (uint8_t)val;
            }
        }
        /* Debio haber hecho al menos 1 iteracion extra */
        assert(iterations >= 1);

        /* Todas las sombras deben estar en [0, 255] */
        for (int j = 0; j < n; j++) {
            assert(shadow_vals[j] <= 255);
        }
        printf("  Caso 2 (loop completo, %d perturbaciones): OK\n", iterations);
    }

    printf("Pruebas de Mitigacion del 256: PASS\n\n");
}

static void test_compression_ratio(void) {
    printf("--- Prueba de Ratios de Compresion ---\n");

    /* Verificar que get_lsb_count * shadow_len / 8 <= M para portadoras del mismo tamanio */
    int test_cases[][2] = {
        {2, 4}, {3, 4}, {4, 2}, {5, 2}, {6, 2}, {7, 2}, {8, 1}, {9, 1}, {10, 1}
    };
    uint32_t M = 90000; /* tamanio tipico: 300x300 */

    for (int t = 0; t < 9; t++) {
        int k = test_cases[t][0];
        int expected_lsb = test_cases[t][1];

        /* Verificar lsb_count */
        int lsb;
        if (k <= 3) lsb = 4;
        else if (k <= 7) lsb = 2;
        else lsb = 1;
        assert(lsb == expected_lsb);

        /* shadow_len = ceil(M / k) */
        uint32_t shadow_len = (M + (uint32_t)k - 1) / (uint32_t)k;

        /* pixeles de portadora necesarios */
        uint32_t needed = shadow_len * (uint32_t)(8 / lsb);

        /* Debe caber en una portadora del mismo tamanio */
        assert(needed <= M);

        printf("  k=%2d: shadow=%5u bytes, LSB%d, carrier_pixels=%6u <= %u: OK\n",
               k, shadow_len, lsb, needed, M);
    }

    printf("Pruebas de Ratios de Compresion: PASS\n\n");
}

static void test_lagrange_recovery(void) {
    printf("--- Prueba de Lagrange Reducido (GF(257)) ---\n");

    /* Polinomio de ejemplo: P(x) = 50 + 51x + 55x^2  (k=3) */
    /* Evaluado en x=1,2,3:
     * x=1 -> y=156
     * x=2 -> y=115
     * x=3 -> y=184
     */
    uint16_t shadow_indices[] = {1, 2, 3};
    uint16_t shadows[] = {156, 115, 184};
    int k = 3;

    uint16_t x[10];
    uint16_t y[10];
    uint16_t coeffs[10];

    for (int i = 0; i < k; i++) {
        x[i] = shadow_indices[i];
        y[i] = shadows[i];
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

        if (j < k - 1) {
            for (int i = 0; i < num_points - 1; i++) {
                uint16_t num_val = gf257_sub(y[i], a_j);
                y[i] = gf257_div(num_val, x[i]);
            }
        }
    }

    assert(coeffs[0] == 50);
    assert(coeffs[1] == 51);
    assert(coeffs[2] == 55);
    printf("  Recuperacion de coeficientes para k=3: OK\n");

    /* Caso k=8 */
    {
        uint16_t orig_coeffs[] = {10, 20, 30, 40, 50, 60, 70, 80};
        uint16_t x_indices[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        uint16_t y_vals[8];
        for (int i = 0; i < 8; i++) {
            y_vals[i] = poly_eval(orig_coeffs, 8, x_indices[i]);
        }

        uint16_t local_x[8];
        uint16_t local_y[8];
        uint16_t rec_coeffs[8];
        for (int i = 0; i < 8; i++) {
            local_x[i] = x_indices[i];
            local_y[i] = y_vals[i];
        }

        for (int j = 0; j < 8; j++) {
            int num_points = 8 - j;
            uint16_t a_j = 0;
            for (int i = 0; i < num_points; i++) {
                uint16_t num = 1;
                uint16_t den = 1;
                for (int m = 0; m < num_points; m++) {
                    if (m == i) continue;
                    uint16_t neg_xm = (uint16_t)((257 - local_x[m]) % 257);
                    num = gf257_mul(num, neg_xm);
                    den = gf257_mul(den, gf257_sub(local_x[i], local_x[m]));
                }
                uint16_t term = gf257_mul(local_y[i], gf257_div(num, den));
                a_j = gf257_add(a_j, term);
            }
            rec_coeffs[j] = a_j;

            if (j < 7) {
                for (int i = 0; i < num_points - 1; i++) {
                    uint16_t num_val = gf257_sub(local_y[i], a_j);
                    local_y[i] = gf257_div(num_val, local_x[i]);
                }
            }
        }

        for (int i = 0; i < 8; i++) {
            assert(rec_coeffs[i] == orig_coeffs[i]);
        }
        printf("  Recuperacion de coeficientes para k=8: OK\n");
    }

    printf("Pruebas de Lagrange Reducido: PASS\n\n");
}

int main(void) {
    printf("===========================================\n");
    printf("   EJECUTANDO PRUEBAS UNITARIAS - FASE 3/4 \n");
    printf("===========================================\n\n");

    gf257_init();

    test_poly_eval();
    test_lsb_roundtrip();
    test_256_mitigation();
    test_compression_ratio();
    test_lagrange_recovery();

    printf("===========================================\n");
    printf("   ¡TODAS LAS PRUEBAS PASARON CON EXITO!  \n");
    printf("===========================================\n");
    return EXIT_SUCCESS;
}
