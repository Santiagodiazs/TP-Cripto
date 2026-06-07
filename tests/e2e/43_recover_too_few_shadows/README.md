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
