#ifndef SSS_H
#define SSS_H

#include "cli.h"

/**
 * Ejecuta el flujo completo de distribucion de secretos:
 *   1. Carga la imagen secreta BMP.
 *   2. Aplica la permutacion XOR con una semilla aleatoria.
 *   3. Genera sombras polinomicas en GF(257) (Shamir / Thien-Lin).
 *   4. Embebe las sombras en los LSBs de las portadoras.
 *   5. Escribe los metadatos (semilla, indice de sombra) en las cabeceras.
 *   6. Guarda las portadoras modificadas.
 *
 * Retorna 0 en caso de exito, -1 en caso de error.
 */
int sss_distribute(const Args *args);

/**
 * Ejecuta el flujo completo de recuperacion del secreto (Fase 4 — stub).
 */
int sss_recover(const Args *args);

#endif /* SSS_H */
