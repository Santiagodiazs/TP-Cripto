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
