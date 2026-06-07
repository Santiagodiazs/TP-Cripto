#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
assert_fail "-d y -r juntos deben fallar" \
    "$SSS" -d -r -secret "$FIXTURES/secret_300.bmp" -k 4 \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
