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
