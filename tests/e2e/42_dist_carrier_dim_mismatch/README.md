# Test 42: portadora con dimensiones distintas al secreto

## Entrada
- Workdir con 7 portadoras 300×300 + 1 portadora 200×200.
- `-d -secret secret_300.bmp -k 8 -n 8 -dir <workdir>` (k=8 exige mismo tamaño).

## Salida esperada
- Exit code: ≠ 0
- Stderr: error indicando que una portadora tiene menos píxeles que los necesarios.
- Justificación: consigna §4.2.5 (esquema 8,n) exige que las portadoras tengan igual
  tamaño que el secreto.

## Validación
El script verifica exit code distinto de cero.
