# Test 26: round-trip k=8 n=8

## Entrada
1. `./sss -d -secret secret_300.bmp -k 8 -n 8 -dir <workdir con 10 portadoras 300×300>`
2. `./sss -r -secret recovered.bmp -k 8 -dir <misma workdir>`

## Salida esperada
- Ambas invocaciones: exit code = 0.
- Tras la distribución, las 8 primeras portadoras de la workdir tienen sombras embebidas
  (bytes 6-7 = seed, bytes 8-9 = shadow_num).
- Tras la recuperación, se crea `recovered.bmp` (300×300, 8 bpp).
- `recovered.bmp` debe coincidir con `secret_300.bmp` en más del 95% de los píxeles
  (la pérdida proviene de la regla del 256, amplificada por el XOR).

## Validación
- `compare_bmp` debe reportar diff ≤ 5%.
- Visualmente, la imagen recuperada debe ser legible (el texto "SECRETO" debe seguir siendo claro).
