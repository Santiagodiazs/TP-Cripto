#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/cli.h"
#include "../src/bmp.h"
#include "../src/math_gf257.h"
#include "../src/permutation.h"
#include "../src/sss.h"

int main(int argc, char *argv[]) {
    gf257_init();
    if (argc < 3) {
        printf("Usage: %s <dir> <k>\n", argv[0]);
        return 1;
    }
    Args args;
    args.mode = MODE_RECOVER;
    args.secret = "recovered_test_cat.bmp";
    args.k = atoi(argv[2]);
    args.dir = argv[1];
    args.n = 0;

    printf("Running recovery diagnostics for k=%d on dir '%s'...\n", args.k, args.dir);
    int ret = sss_recover(&args);
    printf("Recovery returned %d\n", ret);
    return ret;
}
