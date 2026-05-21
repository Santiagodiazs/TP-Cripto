#ifndef MATH_GF257_H
#define MATH_GF257_H

#include <stdint.h>

/**
 * Inicializa la tabla estática de inversos multiplicativos mod 257.
 * Debe ser invocada una única vez apenas arranca el programa.
 */
void gf257_init(void);

/**
 * Suma modular en GF(257).
 * Retorna (a + b) % 257.
 */
uint16_t gf257_add(uint16_t a, uint16_t b);

/**
 * Resta modular en GF(257).
 * Retorna (a - b + 257) % 257.
 */
uint16_t gf257_sub(uint16_t a, uint16_t b);

/**
 * Multiplicacion modular en GF(257).
 * Retorna (a * b) % 257.
 */
uint16_t gf257_mul(uint16_t a, uint16_t b);

/**
 * Division modular en GF(257).
 * Retorna (a * inv[b]) % 257.
 * Aborta o maneja error si b es 0.
 */
uint16_t gf257_div(uint16_t a, uint16_t b);

#endif /* MATH_GF257_H */
