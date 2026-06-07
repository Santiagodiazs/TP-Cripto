# Test 04: k fuera de rango (k=1)

## Entrada
Comando: `./sss -d -secret secret_300.bmp -k 1 -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: -k debe estar entre 2 y 10 (recibido: 1)."
- Justificación: la consigna §4.2.1 establece `2 ≤ k ≤ 10`.

## Validación
El script verifica exit code distinto de cero.
