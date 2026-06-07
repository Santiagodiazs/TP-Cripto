# Test 01: CLI sin argumentos

## Entrada
Comando: `./sss`
(sin ningún argumento)

## Salida esperada
- Exit code: ≠ 0
- Stderr: mensaje "Error: se requieren argumentos." + sintaxis de uso (`Uso: ...`)
- No se crean archivos.

## Validación
El script verifica que `./sss` (sin argumentos) retorne código de salida distinto de cero.
