# Test 41: secreto con bpp inválido

## Entrada
- `-secret secret_24bpp.bmp` (imagen de 24 bpp, no soportada).
- `-d -k 2 -n 8 -dir <workdir>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: mensaje indicando que solo se soportan imágenes de 8 bpp (escala de grises).
- Justificación: consigna §4.2.3 exige 8 bpp.

## Validación
El script verifica exit code distinto de cero.
