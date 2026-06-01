#include <stdio.h>
#include <stdlib.h>
#include "cli.h"
#include "bmp.h"
#include "math_gf257.h"
#include "permutation.h"
#include "sss.h"

int main(int argc, char *argv[]) {
    /* Inicializar aritmética en GF(257) al arrancar */
    gf257_init();

    Args args;
    if (parse_args(argc, argv, &args) != 0) {
        return EXIT_FAILURE;
    }

    int ret;
    if (args.mode == MODE_DISTRIBUTE) {
        ret = sss_distribute(&args);
    } else {
        ret = sss_recover(&args);
    }

    return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

