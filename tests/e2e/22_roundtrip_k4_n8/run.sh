#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
# Setup: portadoras frescas
cp -r "$FIXTURES/carriers_300/." "$WORK/"

# Distribuir
assert_ok "distribución k=4 n=8" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 4 -n 8 -dir "$WORK" \
    || fail=$((fail+1))

# Recuperar (el binario debe encontrar k portadoras con sombras válidas)
assert_ok "recuperación k=4" \
    "$SSS" -r -secret "$WORK/recovered.bmp" -k 4 -dir "$WORK" \
    || fail=$((fail+1))

# Verificar similitud
assert_similar "recuperada ≈ original" \
    "$FIXTURES/secret_300.bmp" "$WORK/recovered.bmp" 5 \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
