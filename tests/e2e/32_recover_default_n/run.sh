#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
# Distribuir SIN -n: el programa debe inferir n del número de portadoras en -dir
cp -r "$FIXTURES/carriers_300/." "$WORK/"

assert_ok "distribución sin -n (n inferido del directorio)" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 4 -dir "$WORK" \
    || fail=$((fail+1))

assert_ok "recuperación con todas las portadoras" \
    "$SSS" -r -secret "$WORK/recovered.bmp" -k 4 -dir "$WORK" \
    || fail=$((fail+1))

assert_similar "recuperada ≈ original" \
    "$FIXTURES/secret_300.bmp" "$WORK/recovered.bmp" 5 \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
