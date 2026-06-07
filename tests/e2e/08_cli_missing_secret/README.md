# Test 08: falta el parámetro -secret

## Entrada
Comando: `./sss -d -k 4 -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: falta el parametro -secret."

## Validación
El script verifica exit code distinto de cero.
