# Test 07: n menor que k

## Entrada
Comando: `./sss -d -secret secret_300.bmp -k 5 -n 3 -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: -n (3) debe ser >= -k (5)."
- Justificación: la consigna §4.2.1 requiere `k ≤ n`.

## Validación
El script verifica exit code distinto de cero.
