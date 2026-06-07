#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
assert_fail "sin -secret debe fallar" \
    "$SSS" -d -k 4 -dir "$WORK" \
    || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
