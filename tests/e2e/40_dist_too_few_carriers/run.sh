#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
# Workdir con SOLO 3 portadoras pero pedimos n=8
cp "$FIXTURES/carriers_300/carrier_01.bmp" \
   "$FIXTURES/carriers_300/carrier_02.bmp" \
   "$FIXTURES/carriers_300/carrier_03.bmp" "$WORK/"

assert_fail "n=8 con solo 3 portadoras debe fallar" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 2 -n 8 -dir "$WORK" \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
