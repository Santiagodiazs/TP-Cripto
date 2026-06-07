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
