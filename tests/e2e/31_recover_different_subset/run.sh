#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
# Distribuir n=8 k=4 y luego recuperar usando las ÚLTIMAS 4 portadoras (5-8)
cp -r "$FIXTURES/carriers_300/." "$WORK/"

assert_ok "distribución k=4 n=8" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 4 -n 8 -dir "$WORK" \
    || fail=$((fail+1))

mkdir -p "$WORK/last4"
cp "$WORK/carrier_05.bmp" "$WORK/carrier_06.bmp" \
   "$WORK/carrier_07.bmp" "$WORK/carrier_08.bmp" "$WORK/last4/"

assert_ok "recuperación con subset distinto (sombras 5-8)" \
    "$SSS" -r -secret "$WORK/recovered.bmp" -k 4 -dir "$WORK/last4" \
    || fail=$((fail+1))

assert_similar "recuperada ≈ original (con subset distinto)" \
    "$FIXTURES/secret_300.bmp" "$WORK/recovered.bmp" 5 \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
