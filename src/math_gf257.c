#include <stdio.h>
#include <stdlib.h>
#include "math_gf257.h"

static uint16_t gf257_inv[257];
static int initialized = 0;

void gf257_init(void) {
    if (initialized) {
        return;
    }
    gf257_inv[0] = 0; /* El 0 no tiene inverso multiplicativo */
    for (int i = 1; i < 257; i++) {
        gf257_inv[i] = 0;
        for (int j = 1; j < 257; j++) {
            if ((i * j) % 257 == 1) {
                gf257_inv[i] = (uint16_t)j;
                break;
            }
        }
    }
    initialized = 1;
}

uint16_t gf257_add(uint16_t a, uint16_t b) {
    return (uint16_t)((a + b) % 257);
}

uint16_t gf257_sub(uint16_t a, uint16_t b) {
    uint32_t val_a = a % 257;
    uint32_t val_b = b % 257;
    return (uint16_t)((val_a + 257 - val_b) % 257);
}

uint16_t gf257_mul(uint16_t a, uint16_t b) {
    uint32_t val_a = a % 257;
    uint32_t val_b = b % 257;
    return (uint16_t)((val_a * val_b) % 257);
}

uint16_t gf257_div(uint16_t a, uint16_t b) {
    uint32_t val_b = b % 257;
    if (val_b == 0) {
        fprintf(stderr, "Error: Division por cero en GF(257).\n");
        exit(EXIT_FAILURE);
    }
    if (!initialized) {
        gf257_init();
    }
    uint32_t val_a = a % 257;
    return (uint16_t)((val_a * gf257_inv[val_b]) % 257);
}
