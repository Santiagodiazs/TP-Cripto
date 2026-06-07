#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
assert_fail "n=3 < k=5 debe fallar" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 5 -n 3 -dir "$WORK" \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
