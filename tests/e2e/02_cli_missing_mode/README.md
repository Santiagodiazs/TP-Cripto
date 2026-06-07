# Test 02: CLI sin modo (-d ni -r)

## Entrada
Comando: `./sss -secret secret_300.bmp -k 4 -dir <vacío>`
(falta el flag `-d` o `-r`)

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: debe indicar -d (distribuir) o -r (recuperar)."
- No se crean archivos.

## Validación
El script verifica exit code distinto de cero.
