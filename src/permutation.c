#include "permutation.h"

/* Estado interno del LCG (48 bits) */
static int64_t lcg_seed = 0;

void lcg_set_seed(int64_t s) {
    lcg_seed = (s ^ 0x5DEECE66DL) & ((1LL << 48) - 1);
}

uint8_t lcg_next_char(void) {
    lcg_seed = (lcg_seed * 0x5DEECE66DL + 0xBL) & ((1LL << 48) - 1);
    return (uint8_t)(lcg_seed >> 40);
}

void permutation_xor_mask(uint8_t *pixels, uint32_t count, uint16_t seed) {
    lcg_set_seed((int64_t)seed);
    for (uint32_t i = 0; i < count; i++) {
        pixels[i] = pixels[i] ^ lcg_next_char();
    }
}
