#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
assert_fail "sss sin argumentos debe fallar" "$SSS" || fail=$((fail+1))
[ "$fail" -eq 0 ] && report 0 || report 1
