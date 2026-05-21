#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/math_gf257.h"
#include "../src/permutation.h"

static void test_math(void) {
    printf("--- Iniciando pruebas de Matematica en GF(257) ---\n");
    gf257_init();

    /* Suma */
    assert(gf257_add(250, 10) == 3);
    assert(gf257_add(200, 57) == 0);
    assert(gf257_add(0, 0) == 0);
    assert(gf257_add(256, 1) == 0); /* 256 + 1 = 257 = 0 */
    printf("  Suma: OK\n");

    /* Resta */
    assert(gf257_sub(3, 10) == 250);
    assert(gf257_sub(0, 57) == 200);
    assert(gf257_sub(10, 10) == 0);
    assert(gf257_sub(0, 1) == 256);
    /* Casos Borde: Restas Negativas */
    assert(gf257_sub(5, 10) == 252);
    assert(gf257_sub(0, 256) == 1);
    printf("  Resta: OK\n");

    /* Multiplicacion */
    assert(gf257_mul(50, 6) == 43); /* 300 % 257 = 43 */
    assert(gf257_mul(256, 2) == 255); /* (256 * 2) = 512 = 255 % 257 */
    assert(gf257_mul(0, 100) == 0);
    /* Caso Borde: Desbordamiento de la Multiplicacion (256 * 256 = 65536) */
    assert(gf257_mul(256, 256) == 1); /* 65536 % 257 = 1 */
    printf("  Multiplicacion: OK\n");

    /* Division e Inversos */
    assert(gf257_div(10, 5) == 2);
    assert(gf257_div(43, 6) == 50);

    /* Caso Borde: Integridad de la Tabla de Inversos */
    /* Verificar que para todo i en [1, 256], i * inv(i) == 1 */
    for (int i = 1; i < 257; i++) {
        uint16_t inv = gf257_div(1, i);
        assert(gf257_mul(i, inv) == 1);
    }
    printf("  Division e Inversos: OK\n");
    printf("Pruebas de Matematica: PASS\n\n");
}

static void test_lcg(void) {
    printf("--- Iniciando pruebas de LCG de 48 bits ---\n");
    
    /* Configurar semilla = 10 */
    lcg_set_seed(10);
    
    /* Generar los primeros 10 valores */
    printf("  Primeros 10 bytes generados con semilla 10:\n  ");
    uint8_t vals[10];
    for (int i = 0; i < 10; i++) {
        vals[i] = lcg_next_char();
        printf("%d\t", vals[i]);
    }
    printf("\n");

    /* Verificar determinismo: la misma semilla debe dar los mismos valores */
    lcg_set_seed(10);
    for (int i = 0; i < 10; i++) {
        assert(lcg_next_char() == vals[i]);
    }
    printf("  Determinismo y Reproducibilidad: OK\n");
    printf("Pruebas de LCG: PASS\n\n");
}

static void test_xor_reversibility(void) {
    printf("--- Iniciando pruebas de Reversibilidad del Enmascaramiento XOR ---\n");

    uint32_t count = 1000;
    uint8_t *original = malloc(count);
    uint8_t *buffer = malloc(count);
    assert(original != NULL && buffer != NULL);

    /* Llenar con datos arbitrarios */
    for (uint32_t i = 0; i < count; i++) {
        original[i] = (uint8_t)(i * 3 + 7);
        buffer[i] = original[i];
    }

    uint16_t seed = 0xABCD;

    /* Primer enmascaramiento */
    permutation_xor_mask(buffer, count, seed);

    /* Verificar que el buffer cambio */
    int changed = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (buffer[i] != original[i]) {
            changed = 1;
            break;
        }
    }
    assert(changed == 1);
    printf("  Primer XOR (Enmascaramiento): OK (el buffer cambio exitosamente)\n");

    /* Segundo enmascaramiento (Desenmascaramiento, debido a auto-reversibilidad) */
    permutation_xor_mask(buffer, count, seed);

    /* Verificar que retorno exactamente a los valores originales */
    for (uint32_t i = 0; i < count; i++) {
        if (buffer[i] != original[i]) {
            fprintf(stderr, "FAIL: Error en reversabilidad en indice %u: orig=%d, recuperado=%d\n",
                    i, original[i], buffer[i]);
            free(original);
            free(buffer);
            exit(EXIT_FAILURE);
        }
    }
    printf("  Segundo XOR (Desenmascaramiento): OK (reversado exacto al original)\n");

    free(original);
    free(buffer);
    printf("Pruebas de XOR Reversibility: PASS\n\n");
}

int main(void) {
    printf("===========================================\n");
    printf("   EJECUTANDO PRUEBAS UNITARIAS - FASE 2   \n");
    printf("===========================================\n\n");

    test_math();
    test_lcg();
    test_xor_reversibility();

    printf("===========================================\n");
    printf("   ¡TODAS LAS PRUEBAS PASARON CON EXITO!  \n");
    printf("===========================================\n");
    return EXIT_SUCCESS;
}
