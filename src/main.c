#include <stdio.h>
#include <stdlib.h>
#include "cli.h"
#include "bmp.h"

/* Stubs — serán implementados en Fases 3 y 4 */
static int distribute(const Args *args) {
    printf("[TODO] Distribución: secret='%s', k=%d, n=%d, dir='%s'\n",
           args->secret, args->k, args->n, args->dir);
    return 0;
}

static int recover(const Args *args) {
    printf("[TODO] Recuperación: secret='%s', k=%d, dir='%s'\n",
           args->secret, args->k, args->dir);
    return 0;
}

int main(int argc, char *argv[]) {
    Args args;
    if (parse_args(argc, argv, &args) != 0) {
        return EXIT_FAILURE;
    }

    int ret;
    if (args.mode == MODE_DISTRIBUTE) {
        ret = distribute(&args);
    } else {
        ret = recover(&args);
    }

    return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
