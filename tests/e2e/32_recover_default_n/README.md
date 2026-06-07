# Test 32: distribución sin -n (n inferido)

## Entrada
1. Workdir con 10 portadoras 300×300.
2. `./sss -d -secret secret_300.bmp -k 4 -dir <workdir>` (sin `-n`).
3. `./sss -r -secret recovered.bmp -k 4 -dir <workdir>`.

## Salida esperada
- Distribución: exit 0. El programa infiere n = 10 (número de BMPs en el directorio).
- Recuperación: exit 0. Imagen similar al original.
- Justificación: la consigna §4.1 dice "Si no se usa, el programa elegirá como valor de n
  la cantidad total de imágenes del directorio."

## Validación
- `compare_bmp` reporta diff ≤ 5%.
