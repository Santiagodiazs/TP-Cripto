#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
# Portadoras de distintos tamaños deben rechazarse durante distribución
# (en distribución, el problema es: una portadora más chica que el secreto)
cp -r "$FIXTURES/carriers_mixed/." "$WORK/"

# Una portadora es 200x200, el secreto es 300x300 → debería fallar en alguna portadora
assert_fail "portadora 200x200 con secreto 300x300 debe fallar" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 8 -n 8 -dir "$WORK" \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
