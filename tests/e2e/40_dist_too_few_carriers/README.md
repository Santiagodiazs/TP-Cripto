# Test 40: portadoras insuficientes

## Entrada
- Workdir con 3 portadoras (carrier_01, 02, 03).
- Comando: `./sss -d -secret secret_300.bmp -k 2 -n 8 -dir <workdir>`

## Salida esperada
- Exit code: ≠ 0
- Stderr: "sss: se necesitan 8 portadoras pero solo se encontraron 3 en '<workdir>'."

## Validación
El script verifica exit code distinto de cero.
