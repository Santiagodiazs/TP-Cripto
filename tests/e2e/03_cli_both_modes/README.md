# Test 03: CLI con -d y -r simultáneos

## Entrada
Comando: `./sss -d -r -secret secret_300.bmp -k 4`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: -d y -r son excluyentes."
- No se crean archivos.

## Validación
El script verifica exit code distinto de cero.
