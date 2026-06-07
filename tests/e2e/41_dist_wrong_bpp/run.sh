#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
# Secreto de 24bpp debe rechazarse
cp -r "$FIXTURES/carriers_300/." "$WORK/"

assert_fail "secret de 24bpp debe fallar" \
    "$SSS" -d -secret "$FIXTURES/secret_24bpp.bmp" -k 2 -n 8 -dir "$WORK" \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
