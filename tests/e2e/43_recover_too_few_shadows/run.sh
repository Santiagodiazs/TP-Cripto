#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
# Distribuir y luego intentar recuperar con menos de k portadoras
cp -r "$FIXTURES/carriers_300/." "$WORK/"

assert_ok "distribución k=4 n=8" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 4 -n 8 -dir "$WORK" \
    || fail=$((fail+1))

# Mover SOLO 3 portadoras al subset (k=4 pero solo hay 3)
mkdir -p "$WORK/few"
cp "$WORK/carrier_01.bmp" "$WORK/carrier_02.bmp" "$WORK/carrier_03.bmp" "$WORK/few/"

assert_fail "recuperar con menos portadoras que k debe fallar" \
    "$SSS" -r -secret "$WORK/recovered.bmp" -k 4 -dir "$WORK/few" \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
