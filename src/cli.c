#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"

void print_usage(const char *prog) {
    fprintf(stderr,
        "Uso: %s {-d | -r} -secret <imagen.bmp> -k <numero> [-n <numero>] [-dir <directorio>]\n"
        "\n"
        "  -d              Distribuir: genera n sombras a partir de la imagen secreta.\n"
        "  -r              Recuperar: reconstruye la imagen secreta desde las portadoras.\n"
        "  -secret <img>   Ruta a la imagen BMP secreta (requerido).\n"
        "  -k <num>        Numero minimo de sombras para recuperar (2 <= k <= 10, requerido).\n"
        "  -n <num>        Total de sombras a generar (>= k; solo para -d; inferido si se omite).\n"
        "  -dir <dir>      Directorio que contiene las portadoras (default: directorio actual).\n",
        prog);
}

int parse_args(int argc, char *argv[], Args *out) {
    if (argc < 2) {
        fprintf(stderr, "Error: se requieren argumentos.\n");
        print_usage(argv[0]);
        return -1;
    }

    /* Initialize defaults */
    out->mode   = 0;
    out->secret = NULL;
    out->k      = 0;
    out->n      = 0;
    out->dir    = ".";

    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            if (out->mode != 0) {
                fprintf(stderr, "Error: -d y -r son excluyentes.\n");
                print_usage(argv[0]);
                return -1;
            }
            out->mode = MODE_DISTRIBUTE;
        } else if (strcmp(argv[i], "-r") == 0) {
            if (out->mode != 0) {
                fprintf(stderr, "Error: -d y -r son excluyentes.\n");
                print_usage(argv[0]);
                return -1;
            }
            out->mode = MODE_RECOVER;
        } else if (strcmp(argv[i], "-secret") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: -secret requiere un argumento.\n");
                print_usage(argv[0]);
                return -1;
            }
            out->secret = argv[i];
        } else if (strcmp(argv[i], "-k") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: -k requiere un argumento.\n");
                print_usage(argv[0]);
                return -1;
            }
            char *end;
            long val = strtol(argv[i], &end, 10);
            if (*end != '\0') {
                fprintf(stderr, "Error: -k debe ser un numero entero (recibido: '%s').\n", argv[i]);
                print_usage(argv[0]);
                return -1;
            }
            if (val < 2 || val > 10) {
                fprintf(stderr, "Error: -k debe estar entre 2 y 10 (recibido: %ld).\n", val);
                print_usage(argv[0]);
                return -1;
            }
            out->k = (int)val;
        } else if (strcmp(argv[i], "-n") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: -n requiere un argumento.\n");
                print_usage(argv[0]);
                return -1;
            }
            char *end;
            long val = strtol(argv[i], &end, 10);
            if (*end != '\0' || val < 2) {
                fprintf(stderr, "Error: -n debe ser un entero >= 2 (recibido: '%s').\n", argv[i]);
                print_usage(argv[0]);
                return -1;
            }
            out->n = (int)val;
        } else if (strcmp(argv[i], "-dir") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error: -dir requiere un argumento.\n");
                print_usage(argv[0]);
                return -1;
            }
            out->dir = argv[i];
        } else {
            fprintf(stderr, "Error: parametro desconocido '%s'.\n", argv[i]);
            print_usage(argv[0]);
            return -1;
        }
    }

    /* --- Post-parse validations --- */

    if (out->mode == 0) {
        fprintf(stderr, "Error: debe indicar -d (distribuir) o -r (recuperar).\n");
        print_usage(argv[0]);
        return -1;
    }
    if (out->secret == NULL) {
        fprintf(stderr, "Error: falta el parametro -secret.\n");
        print_usage(argv[0]);
        return -1;
    }
    if (out->k == 0) {
        fprintf(stderr, "Error: falta el parametro -k.\n");
        print_usage(argv[0]);
        return -1;
    }
    if (out->n != 0 && out->n < out->k) {
        fprintf(stderr, "Error: -n (%d) debe ser >= -k (%d).\n", out->n, out->k);
        print_usage(argv[0]);
        return -1;
    }
    if (out->mode == MODE_RECOVER && out->n != 0) {
        /* -n is ignored in recovery mode; warn but don't fail */
        fprintf(stderr, "Advertencia: -n es ignorado en modo recuperacion.\n");
        out->n = 0;
    }

    return 0;
}
