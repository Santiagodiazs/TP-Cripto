#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
assert_fail "k=1 debe fallar (mínimo k=2)" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 1 -dir "$WORK" \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
