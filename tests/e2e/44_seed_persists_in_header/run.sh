#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
# Validar que después de distribución los bytes 6-9 de las portadoras tengan seed y shadow_num grabados
cp -r "$FIXTURES/carriers_300/." "$WORK/"

assert_ok "distribución k=4 n=8" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 4 -n 8 -dir "$WORK" \
    || fail=$((fail+1))

# La primera portadora debe tener:
#   - bytes 6-7: seed (no necesariamente conocido, pero NO ser 0x00 0x00)
#   - bytes 8-9: shadow_num = 1 (es decir, byte 8 = 0x01, byte 9 = 0x00)
b6=$(xxd -s 6 -l 1 -p "$WORK/carrier_01.bmp")
b7=$(xxd -s 7 -l 1 -p "$WORK/carrier_01.bmp")
b8=$(xxd -s 8 -l 1 -p "$WORK/carrier_01.bmp")
b9=$(xxd -s 9 -l 1 -p "$WORK/carrier_01.bmp")

# shadow_num = 1 (LE) → byte 8 = 01, byte 9 = 00
if [ "$b8" = "01" ] && [ "$b9" = "00" ]; then
    echo "  [ok]  shadow_num=1 grabado en bytes 8-9 de carrier_01.bmp ($b8 $b9)"
else
    echo "  [FAIL] bytes 8-9 de carrier_01.bmp son $b8 $b9, esperaba 01 00" >&2
    fail=$((fail+1))
fi

# La seed no debe ser 0x0000 (probabilidad despreciable)
if [ "$b6" = "00" ] && [ "$b7" = "00" ]; then
    echo "  [FAIL] seed (bytes 6-7) es 00 00; debería ser un valor pseudoaleatorio" >&2
    fail=$((fail+1))
else
    echo "  [ok]  seed presente en bytes 6-7 ($b6 $b7)"
fi

# Verificar que la octava portadora tenga shadow_num = 8
b8_last=$(xxd -s 8 -l 1 -p "$WORK/carrier_08.bmp")
b9_last=$(xxd -s 9 -l 1 -p "$WORK/carrier_08.bmp")
if [ "$b8_last" = "08" ] && [ "$b9_last" = "00" ]; then
    echo "  [ok]  shadow_num=8 grabado en bytes 8-9 de carrier_08.bmp ($b8_last $b9_last)"
else
    echo "  [FAIL] bytes 8-9 de carrier_08.bmp son $b8_last $b9_last, esperaba 08 00" >&2
    fail=$((fail+1))
fi
[ "$fail" -eq 0 ] && report 0 || report 1
