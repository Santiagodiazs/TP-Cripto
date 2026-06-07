#!/usr/bin/env bash
# Runner maestro: ejecuta todos los tests e2e y reporta resumen.

HERE=$(cd "$(dirname "$0")" && pwd)
cd "$HERE"

# Garantizar fixtures + binarios
if [ ! -d "fixtures/carriers_300" ] || [ ! -f "fixtures/secret_300.bmp" ]; then
    echo "[setup] Generando fixtures..."
    python3 setup_fixtures.py
fi

REPO=$(cd "$HERE/../.." && pwd)
if [ ! -x "$REPO/sss" ]; then
    echo "[setup] Compilando sss..."
    (cd "$REPO" && make >/dev/null 2>&1)
fi
if [ ! -x "$REPO/tests/compare_bmp" ]; then
    echo "[setup] Compilando compare_bmp..."
    (cd "$REPO" && make compare_bmp >/dev/null 2>&1)
fi

# Si nos pasan argumentos, son los tests a correr; si no, corremos todos.
if [ $# -gt 0 ]; then
    TESTS=("$@")
else
    TESTS=()
    for d in [0-9]*/; do
        TESTS+=("${d%/}")
    done
fi

pass=0
fail=0
failed_tests=()

echo ""
echo "================================================================"
echo "  Ejecutando ${#TESTS[@]} tests e2e"
echo "================================================================"
echo ""

for t in "${TESTS[@]}"; do
    if [ ! -f "$t/run.sh" ]; then
        echo "SKIP: $t (no run.sh)"
        continue
    fi
    echo "--- $t ---"
    if bash "$t/run.sh"; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1))
        failed_tests+=("$t")
    fi
    echo ""
done

echo "================================================================"
echo "  Resumen: $pass passed, $fail failed (de ${#TESTS[@]} totales)"
echo "================================================================"
if [ $fail -gt 0 ]; then
    echo "Tests fallidos:"
    for ft in "${failed_tests[@]}"; do
        echo "  - $ft"
    done
    exit 1
fi
exit 0
