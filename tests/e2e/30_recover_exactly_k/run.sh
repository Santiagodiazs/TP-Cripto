#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
# Distribuir con n=8 k=4, luego dejar SOLO 4 portadoras en otra workdir y recuperar
cp -r "$FIXTURES/carriers_300/." "$WORK/"

assert_ok "distribución k=4 n=8" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 4 -n 8 -dir "$WORK" \
    || fail=$((fail+1))

# Dejar solo las primeras 4 portadoras
mkdir -p "$WORK/subset"
cp "$WORK/carrier_01.bmp" "$WORK/carrier_02.bmp" \
   "$WORK/carrier_03.bmp" "$WORK/carrier_04.bmp" "$WORK/subset/"

assert_ok "recuperación con exactamente k=4 portadoras" \
    "$SSS" -r -secret "$WORK/recovered.bmp" -k 4 -dir "$WORK/subset" \
    || fail=$((fail+1))

assert_similar "recuperada ≈ original" \
    "$FIXTURES/secret_300.bmp" "$WORK/recovered.bmp" 5 \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
