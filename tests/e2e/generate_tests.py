#!/usr/bin/env python3
"""
Genera los 25 tests e2e como carpetas individuales con `run.sh` + `README.md`.

Idempotente: sobreescribe los archivos generados sin tocar otros.

Cada test queda autocontenido: el script `run.sh` arma su workdir, corre
`sss` con los parámetros del caso, verifica el resultado, y reporta
PASS/FAIL.

El `README.md` describe en lenguaje natural la entrada y la salida esperada
para que un humano pueda validar el test sin leer el script.
"""
from pathlib import Path
from textwrap import dedent

HERE = Path(__file__).resolve().parent

# Plantilla común para todos los run.sh
RUN_TEMPLATE = """#!/usr/bin/env bash
TEST_DIR=$(cd "$(dirname "$0")" && pwd)
source "$TEST_DIR/../lib.sh"
ensure_binaries
init_workdir

fail=0
{body}
[ "$fail" -eq 0 ] && report 0 || report 1
"""

def write_test(name, body, readme):
    d = HERE / name
    d.mkdir(parents=True, exist_ok=True)
    (d / "run.sh").write_text(RUN_TEMPLATE.format(body=body.strip()))
    (d / "run.sh").chmod(0o755)
    (d / "README.md").write_text(readme.strip() + "\n")

# ---------------------------------------------------------------------------
# CLI: validación de argumentos
# ---------------------------------------------------------------------------

write_test("01_cli_no_args",
    body=r'''
assert_fail "sss sin argumentos debe fallar" "$SSS" || fail=$((fail+1))
''',
    readme=r"""
# Test 01: CLI sin argumentos

## Entrada
Comando: `./sss`
(sin ningún argumento)

## Salida esperada
- Exit code: ≠ 0
- Stderr: mensaje "Error: se requieren argumentos." + sintaxis de uso (`Uso: ...`)
- No se crean archivos.

## Validación
El script verifica que `./sss` (sin argumentos) retorne código de salida distinto de cero.
""")

write_test("02_cli_missing_mode",
    body=r'''
assert_fail "sin -d/-r debe fallar" \
    "$SSS" -secret "$FIXTURES/secret_300.bmp" -k 4 -dir "$WORK" \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 02: CLI sin modo (-d ni -r)

## Entrada
Comando: `./sss -secret secret_300.bmp -k 4 -dir <vacío>`
(falta el flag `-d` o `-r`)

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: debe indicar -d (distribuir) o -r (recuperar)."
- No se crean archivos.

## Validación
El script verifica exit code distinto de cero.
""")

write_test("03_cli_both_modes",
    body=r'''
assert_fail "-d y -r juntos deben fallar" \
    "$SSS" -d -r -secret "$FIXTURES/secret_300.bmp" -k 4 \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 03: CLI con -d y -r simultáneos

## Entrada
Comando: `./sss -d -r -secret secret_300.bmp -k 4`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: -d y -r son excluyentes."
- No se crean archivos.

## Validación
El script verifica exit code distinto de cero.
""")

write_test("04_cli_k_too_low",
    body=r'''
assert_fail "k=1 debe fallar (mínimo k=2)" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 1 -dir "$WORK" \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 04: k fuera de rango (k=1)

## Entrada
Comando: `./sss -d -secret secret_300.bmp -k 1 -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: -k debe estar entre 2 y 10 (recibido: 1)."
- Justificación: la consigna §4.2.1 establece `2 ≤ k ≤ 10`.

## Validación
El script verifica exit code distinto de cero.
""")

write_test("05_cli_k_too_high",
    body=r'''
assert_fail "k=11 debe fallar (máximo k=10)" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 11 -dir "$WORK" \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 05: k fuera de rango (k=11)

## Entrada
Comando: `./sss -d -secret secret_300.bmp -k 11 -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: -k debe estar entre 2 y 10 (recibido: 11)."

## Validación
El script verifica exit code distinto de cero.
""")

write_test("06_cli_k_non_numeric",
    body=r'''
assert_fail "k no numérico debe fallar" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k abc -dir "$WORK" \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 06: k no numérico

## Entrada
Comando: `./sss -d -secret secret_300.bmp -k abc -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: -k debe ser un numero entero (recibido: 'abc')."

## Validación
El script verifica exit code distinto de cero.
""")

write_test("07_cli_n_less_than_k",
    body=r'''
assert_fail "n=3 < k=5 debe fallar" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 5 -n 3 -dir "$WORK" \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 07: n menor que k

## Entrada
Comando: `./sss -d -secret secret_300.bmp -k 5 -n 3 -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: -n (3) debe ser >= -k (5)."
- Justificación: la consigna §4.2.1 requiere `k ≤ n`.

## Validación
El script verifica exit code distinto de cero.
""")

write_test("08_cli_missing_secret",
    body=r'''
assert_fail "sin -secret debe fallar" \
    "$SSS" -d -k 4 -dir "$WORK" \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 08: falta el parámetro -secret

## Entrada
Comando: `./sss -d -k 4 -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: falta el parametro -secret."

## Validación
El script verifica exit code distinto de cero.
""")

write_test("09_cli_missing_k",
    body=r'''
assert_fail "sin -k debe fallar" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -dir "$WORK" \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 09: falta el parámetro -k

## Entrada
Comando: `./sss -d -secret secret_300.bmp -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: falta el parametro -k."

## Validación
El script verifica exit code distinto de cero.
""")

write_test("10_cli_unknown_flag",
    body=r'''
assert_fail "flag desconocido debe fallar" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 4 -dir "$WORK" --foo \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 10: parámetro desconocido

## Entrada
Comando: `./sss -d -secret secret_300.bmp -k 4 -dir <vacío> --foo`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: parametro desconocido '--foo'."

## Validación
El script verifica exit code distinto de cero.
""")

# ---------------------------------------------------------------------------
# Round-trips k=2..10
# ---------------------------------------------------------------------------

def roundtrip_test(num, k, n, max_diff_pct):
    name = f"{num}_roundtrip_k{k}_n{n}"
    write_test(name,
        body=fr'''
# Setup: portadoras frescas
cp -r "$FIXTURES/carriers_300/." "$WORK/"

# Distribuir
assert_ok "distribución k={k} n={n}" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k {k} -n {n} -dir "$WORK" \
    || fail=$((fail+1))

# Recuperar (el binario debe encontrar k portadoras con sombras válidas)
assert_ok "recuperación k={k}" \
    "$SSS" -r -secret "$WORK/recovered.bmp" -k {k} -dir "$WORK" \
    || fail=$((fail+1))

# Verificar similitud
assert_similar "recuperada ≈ original" \
    "$FIXTURES/secret_300.bmp" "$WORK/recovered.bmp" {max_diff_pct} \
    || fail=$((fail+1))
''',
        readme=fr"""
# Test {num}: round-trip k={k} n={n}

## Entrada
1. `./sss -d -secret secret_300.bmp -k {k} -n {n} -dir <workdir con 10 portadoras 300×300>`
2. `./sss -r -secret recovered.bmp -k {k} -dir <misma workdir>`

## Salida esperada
- Ambas invocaciones: exit code = 0.
- Tras la distribución, las {n} primeras portadoras de la workdir tienen sombras embebidas
  (bytes 6-7 = seed, bytes 8-9 = shadow_num).
- Tras la recuperación, se crea `recovered.bmp` (300×300, 8 bpp).
- `recovered.bmp` debe coincidir con `secret_300.bmp` en más del {100 - max_diff_pct}% de los píxeles
  (la pérdida proviene de la regla del 256, amplificada por el XOR).

## Validación
- `compare_bmp` debe reportar diff ≤ {max_diff_pct}%.
- Visualmente, la imagen recuperada debe ser legible (el texto "SECRETO" debe seguir siendo claro).
""")

# k=2..10, max diff esperado generoso (la regla del 256 da ~3% para n=8 y crece con n)
roundtrip_test("20",  2, 8, 5)
roundtrip_test("21",  3, 8, 5)
roundtrip_test("22",  4, 8, 5)
roundtrip_test("23",  5, 8, 5)
roundtrip_test("24",  6, 8, 5)
roundtrip_test("25",  7, 8, 5)
roundtrip_test("26",  8, 8, 5)
roundtrip_test("27",  9, 10, 5)
roundtrip_test("28", 10, 10, 5)

# ---------------------------------------------------------------------------
# Recovery edge cases
# ---------------------------------------------------------------------------

write_test("30_recover_exactly_k",
    body=r'''
# Distribuir con n=8 k=4, luego dejar SOLO 4 portadoras en otra workdir y recuperar
cp -r "$FIXTURES/carriers_300/." "$WORK/"

assert_ok "distribución k=4 n=8" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 4 -n 8 -dir "$WORK" \
    || fail=$((fail+1))

# Dejar solo las primeras 4 portadoras
mkdir -p "$WORK/subset"
cp "$WORK/carrier_01.bmp" "$WORK/carrier_02.bmp" \
   "$WORK/carrier_03.bmp" "$WORK/carrier_04.bmp" "$WORK/subset/"

assert_ok "recuperación con exactamente k=4 portadoras" \
    "$SSS" -r -secret "$WORK/recovered.bmp" -k 4 -dir "$WORK/subset" \
    || fail=$((fail+1))

assert_similar "recuperada ≈ original" \
    "$FIXTURES/secret_300.bmp" "$WORK/recovered.bmp" 5 \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 30: recuperación con exactamente k portadoras

## Entrada
1. `./sss -d -secret secret_300.bmp -k 4 -n 8 -dir <workdir con 10 portadoras>` (produce 8 sombras)
2. Mover las primeras 4 portadoras a `subset/`
3. `./sss -r -secret recovered.bmp -k 4 -dir <subset>` (solo 4 portadoras disponibles)

## Salida esperada
- Distribución: exit 0.
- Recuperación: exit 0, `recovered.bmp` similar al original.
- Valida que `k` sombras son SUFICIENTES (no se necesitan las `n`).

## Validación
- `compare_bmp` reporta diff ≤ 5%.
""")

write_test("31_recover_different_subset",
    body=r'''
# Distribuir n=8 k=4 y luego recuperar usando las ÚLTIMAS 4 portadoras (5-8)
cp -r "$FIXTURES/carriers_300/." "$WORK/"

assert_ok "distribución k=4 n=8" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 4 -n 8 -dir "$WORK" \
    || fail=$((fail+1))

mkdir -p "$WORK/last4"
cp "$WORK/carrier_05.bmp" "$WORK/carrier_06.bmp" \
   "$WORK/carrier_07.bmp" "$WORK/carrier_08.bmp" "$WORK/last4/"

assert_ok "recuperación con subset distinto (sombras 5-8)" \
    "$SSS" -r -secret "$WORK/recovered.bmp" -k 4 -dir "$WORK/last4" \
    || fail=$((fail+1))

assert_similar "recuperada ≈ original (con subset distinto)" \
    "$FIXTURES/secret_300.bmp" "$WORK/recovered.bmp" 5 \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 31: recuperación con subset distinto

## Entrada
1. Distribuir `-d -k 4 -n 8` → 8 sombras (en portadoras 1..8).
2. Mover sólo las portadoras 5-8 a `last4/`.
3. `./sss -r -secret recovered.bmp -k 4 -dir last4`.

## Salida esperada
- Ambas invocaciones exit 0.
- La imagen recuperada usando un subset distinto (sombras 5-8 en vez de 1-4) debe ser equivalente
  al original. Esto valida la propiedad del esquema (k,n): **cualquier** subconjunto de k sombras
  permite la recuperación.

## Validación
- `compare_bmp` reporta diff ≤ 5%.
""")

write_test("32_recover_default_n",
    body=r'''
# Distribuir SIN -n: el programa debe inferir n del número de portadoras en -dir
cp -r "$FIXTURES/carriers_300/." "$WORK/"

assert_ok "distribución sin -n (n inferido del directorio)" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 4 -dir "$WORK" \
    || fail=$((fail+1))

assert_ok "recuperación con todas las portadoras" \
    "$SSS" -r -secret "$WORK/recovered.bmp" -k 4 -dir "$WORK" \
    || fail=$((fail+1))

assert_similar "recuperada ≈ original" \
    "$FIXTURES/secret_300.bmp" "$WORK/recovered.bmp" 5 \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 32: distribución sin -n (n inferido)

## Entrada
1. Workdir con 10 portadoras 300×300.
2. `./sss -d -secret secret_300.bmp -k 4 -dir <workdir>` (sin `-n`).
3. `./sss -r -secret recovered.bmp -k 4 -dir <workdir>`.

## Salida esperada
- Distribución: exit 0. El programa infiere n = 10 (número de BMPs en el directorio).
- Recuperación: exit 0. Imagen similar al original.
- Justificación: la consigna §4.1 dice "Si no se usa, el programa elegirá como valor de n
  la cantidad total de imágenes del directorio."

## Validación
- `compare_bmp` reporta diff ≤ 5%.
""")

# ---------------------------------------------------------------------------
# Casos de error y formato
# ---------------------------------------------------------------------------

write_test("40_dist_too_few_carriers",
    body=r'''
# Workdir con SOLO 3 portadoras pero pedimos n=8
cp "$FIXTURES/carriers_300/carrier_01.bmp" \
   "$FIXTURES/carriers_300/carrier_02.bmp" \
   "$FIXTURES/carriers_300/carrier_03.bmp" "$WORK/"

assert_fail "n=8 con solo 3 portadoras debe fallar" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 2 -n 8 -dir "$WORK" \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 40: portadoras insuficientes

## Entrada
- Workdir con 3 portadoras (carrier_01, 02, 03).
- Comando: `./sss -d -secret secret_300.bmp -k 2 -n 8 -dir <workdir>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "sss: se necesitan 8 portadoras pero solo se encontraron 3 en '<workdir>'."

## Validación
El script verifica exit code distinto de cero.
""")

write_test("41_dist_wrong_bpp",
    body=r'''
# Secreto de 24bpp debe rechazarse
cp -r "$FIXTURES/carriers_300/." "$WORK/"

assert_fail "secret de 24bpp debe fallar" \
    "$SSS" -d -secret "$FIXTURES/secret_24bpp.bmp" -k 2 -n 8 -dir "$WORK" \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 41: secreto con bpp inválido

## Entrada
- `-secret secret_24bpp.bmp` (imagen de 24 bpp, no soportada).
- `-d -k 2 -n 8 -dir <workdir>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: mensaje indicando que solo se soportan imágenes de 8 bpp (escala de grises).
- Justificación: consigna §4.2.3 exige 8 bpp.

## Validación
El script verifica exit code distinto de cero.
""")

write_test("42_dist_carrier_dim_mismatch",
    body=r'''
# Portadoras de distintos tamaños deben rechazarse durante distribución
# (en distribución, el problema es: una portadora más chica que el secreto)
cp -r "$FIXTURES/carriers_mixed/." "$WORK/"

# Una portadora es 200x200, el secreto es 300x300 → debería fallar en alguna portadora
assert_fail "portadora 200x200 con secreto 300x300 debe fallar" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 8 -n 8 -dir "$WORK" \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 42: portadora con dimensiones distintas al secreto

## Entrada
- Workdir con 7 portadoras 300×300 + 1 portadora 200×200.
- `-d -secret secret_300.bmp -k 8 -n 8 -dir <workdir>` (k=8 exige mismo tamaño).

## Salida esperada
- Exit code: ≠ 0
- Stderr: error indicando que una portadora tiene menos píxeles que los necesarios.
- Justificación: consigna §4.2.5 (esquema 8,n) exige que las portadoras tengan igual
  tamaño que el secreto.

## Validación
El script verifica exit code distinto de cero.
""")

write_test("43_recover_too_few_shadows",
    body=r'''
# Distribuir y luego intentar recuperar con menos de k portadoras
cp -r "$FIXTURES/carriers_300/." "$WORK/"

assert_ok "distribución k=4 n=8" \
    "$SSS" -d -secret "$FIXTURES/secret_300.bmp" -k 4 -n 8 -dir "$WORK" \
    || fail=$((fail+1))

# Mover SOLO 3 portadoras al subset (k=4 pero solo hay 3)
mkdir -p "$WORK/few"
cp "$WORK/carrier_01.bmp" "$WORK/carrier_02.bmp" "$WORK/carrier_03.bmp" "$WORK/few/"

assert_fail "recuperar con menos portadoras que k debe fallar" \
    "$SSS" -r -secret "$WORK/recovered.bmp" -k 4 -dir "$WORK/few" \
    || fail=$((fail+1))
''',
    readme=r"""
# Test 43: recuperación con menos de k portadoras

## Entrada
1. Distribuir `-d -k 4 -n 8` (genera 8 sombras).
2. Mover sólo 3 portadoras a `few/`.
3. `./sss -r -secret recovered.bmp -k 4 -dir few`.

## Salida esperada
- Distribución: exit 0.
- Recuperación: exit ≠ 0, mensaje "se necesitan al menos 4 portadoras válidas..."

## Validación
El script verifica que el `-r` falle.
""")

write_test("44_seed_persists_in_header",
    body=r'''
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
''',
    readme=r"""
# Test 44: seed y shadow_num persistidos en cabecera

## Entrada
- Distribuir `-d -k 4 -n 8 -dir <workdir>`.

## Salida esperada
- Distribución: exit 0.
- Bytes 6-7 de cada portadora resultante contienen la **semilla** (no `0x00 0x00`).
- Bytes 8-9 contienen el **shadow_num** en little-endian (`carrier_01.bmp` → `01 00`,
  `carrier_08.bmp` → `08 00`).
- Justificación: consigna §4.2.5 prescribe exactamente esta ubicación y endianness.

## Validación
- `xxd` inspecciona bytes 6-9 y se compara contra los valores esperados.
- La seed se verifica que no sea cero (probabilidad despreciable de colisión en LCG con
  semilla aleatoria de 16 bits).
""")

print("Tests generados:")
for d in sorted(HERE.iterdir()):
    if d.is_dir() and (d / "run.sh").exists():
        print(f"  {d.name}/")
