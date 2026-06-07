# Test 06: k no numérico

## Entrada
Comando: `./sss -d -secret secret_300.bmp -k abc -dir <vacío>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "Error: -k debe ser un numero entero (recibido: 'abc')."

## Validación
El script verifica exit code distinto de cero.
