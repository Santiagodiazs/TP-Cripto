#ifndef PERMUTATION_H
#define PERMUTATION_H

#include <stdint.h>

/**
 * Inicializa la semilla del generador congruencial lineal (LCG) de 48 bits.
 */
void lcg_set_seed(int64_t s);

/**
 * Genera el siguiente byte pseudoaleatorio en el rango [0, 255].
 * Utiliza los 8 bits superiores del estado interno de 48 bits.
 */
uint8_t lcg_next_char(void);

/**
 * Aplica una mascara XOR byte a byte sobre el buffer de pixeles.
 * Como el XOR es auto-reversible (A ^ B ^ B = A), llamar a esta funcion
 * con la misma semilla realiza tanto el enmascarado (distribucion)
 * como el desenmascarado (recuperacion).
 *
 * pixels: el buffer de pixeles a enmascarar in-place
 * count: la cantidad de bytes/pixeles en el buffer
 * seed: la semilla de 16 bits para inicializar el LCG
 */
void permutation_xor_mask(uint8_t *pixels, uint32_t count, uint16_t seed);

#endif /* PERMUTATION_H */
