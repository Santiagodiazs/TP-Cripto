# Test 05: k fuera de rango (k=11)

## Entrada
Comando: `./sss -d -secret secret_300.bmp -k 11 -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: -k debe estar entre 2 y 10 (recibido: 11)."

## Validación
El script verifica exit code distinto de cero.
