#ifndef CLI_H
#define CLI_H

#define MODE_DISTRIBUTE 1
#define MODE_RECOVER    2

typedef struct {
    int   mode;    /* MODE_DISTRIBUTE or MODE_RECOVER */
    char *secret;  /* path to secret BMP image        */
    int   k;       /* minimum shadows needed (2..10)  */
    int   n;       /* total shadows; 0 = infer        */
    char *dir;     /* directory with carriers         */
} Args;

/**
 * Parse argc/argv into *out.
 * Returns 0 on success, -1 on error (already printed message + usage).
 */
int  parse_args(int argc, char *argv[], Args *out);
void print_usage(const char *prog);

#endif /* CLI_H */
