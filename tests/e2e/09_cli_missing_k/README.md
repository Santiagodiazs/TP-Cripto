# Test 09: falta el parámetro -k

## Entrada
Comando: `./sss -d -secret secret_300.bmp -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: falta el parametro -k."

## Validación
El script verifica exit code distinto de cero.
