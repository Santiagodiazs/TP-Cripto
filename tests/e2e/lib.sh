#!/usr/bin/env bash
# Helpers para los tests e2e. Se "source"-ea desde cada test.

# Resolver paths absolutos. $TEST_DIR debe estar definido por el caller.
if [ -z "$TEST_DIR" ]; then
    echo "lib.sh: \$TEST_DIR no está definido" >&2
    exit 2
fi

REPO_ROOT=$(cd "$TEST_DIR/../../.." && pwd)
SSS="$REPO_ROOT/sss"
COMPARE_BMP="$REPO_ROOT/tests/compare_bmp"
FIXTURES="$REPO_ROOT/tests/e2e/fixtures"
WORK="$TEST_DIR/_work"

# Limpia y crea $WORK como directorio de trabajo aislado para el test
init_workdir() {
    rm -rf "$WORK"
    mkdir -p "$WORK"
}

# Verifica que sss y compare_bmp existan, los compila si no
ensure_binaries() {
    if [ ! -x "$SSS" ]; then
        (cd "$REPO_ROOT" && make >/dev/null 2>&1)
    fi
    if [ ! -x "$COMPARE_BMP" ]; then
        (cd "$REPO_ROOT" && make compare_bmp >/dev/null 2>&1)
    fi
    if [ ! -x "$SSS" ] || [ ! -x "$COMPARE_BMP" ]; then
        echo "ERROR: no se pudo encontrar/compilar sss o compare_bmp" >&2
        exit 2
    fi
}

# Imprime PASS/FAIL del test usando el código de salida pasado
report() {
    local rc=$1
    if [ "$rc" -eq 0 ]; then
        echo "PASS: $(basename "$TEST_DIR")"
    else
        echo "FAIL: $(basename "$TEST_DIR")"
    fi
    exit "$rc"
}

# Devuelve el porcentaje de píxeles diferentes (entero, redondeado hacia abajo)
# entre dos BMPs, usando compare_bmp.
diff_pct() {
    local a=$1 b=$2
    "$COMPARE_BMP" "$a" "$b" 2>/dev/null \
        | awk -F'[()%]' '/Pixeles dif/ { gsub(/^[ \t]+/, "", $2); print int($2+0); exit }'
}

# Aserción: comando debe terminar con éxito (exit 0)
assert_ok() {
    local desc=$1; shift
    if "$@"; then
        echo "  [ok]  $desc"
        return 0
    else
        echo "  [FAIL] $desc (exit $?)" >&2
        return 1
    fi
}

# Aserción: comando debe fallar (exit != 0)
assert_fail() {
    local desc=$1; shift
    if "$@" >/dev/null 2>&1; then
        echo "  [FAIL] $desc (debió fallar y no falló)" >&2
        return 1
    else
        echo "  [ok]  $desc (falló como se esperaba)"
        return 0
    fi
}

# Aserción: diff_pct entre dos BMPs ≤ max_pct
assert_similar() {
    local desc=$1 a=$2 b=$3 max_pct=${4:-5}
    local pct
    pct=$(diff_pct "$a" "$b")
    if [ -z "$pct" ]; then
        echo "  [FAIL] $desc (compare_bmp no devolvió porcentaje)" >&2
        return 1
    fi
    if [ "$pct" -le "$max_pct" ]; then
        echo "  [ok]  $desc (diff ${pct}% ≤ ${max_pct}%)"
        return 0
    else
        echo "  [FAIL] $desc (diff ${pct}% > ${max_pct}%)" >&2
        return 1
    fi
}
