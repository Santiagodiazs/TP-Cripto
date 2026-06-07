# Test 10: parámetro desconocido

## Entrada
Comando: `./sss -d -secret secret_300.bmp -k 4 -dir <vacío> --foo`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: parametro desconocido '--foo'."

## Validación
El script verifica exit code distinto de cero.
