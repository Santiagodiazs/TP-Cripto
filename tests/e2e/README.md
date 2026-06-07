# Tests end-to-end

Suite de tests de integración que validan toda la funcionalidad solicitada por
la consigna del TP. Cada test es una carpeta autocontenida con su `run.sh`
(ejecuta) y `README.md` (describe entrada y salida esperada).

## Organización

```
tests/e2e/
├── README.md                  (este archivo)
├── setup_fixtures.py          (genera imágenes BMP de prueba con PIL)
├── lib.sh                     (helpers de bash compartidos)
├── run_all.sh                 (corre todos los tests y reporta)
├── generate_tests.py          (regenera todos los test/run.sh y README.md desde una sola fuente)
├── fixtures/                  (BMPs y archivos auxiliares)
│   ├── secret_300.bmp         (300×300 8bpp, texto "SECRETO")
│   ├── secret_100.bmp         (100×100 8bpp)
│   ├── secret_24bpp.bmp       (100×100 24bpp, para test de bpp inválido)
│   ├── not_a_bmp.txt          (archivo no-BMP)
│   ├── carriers_300/          (10 portadoras 300×300 8bpp)
│   ├── carriers_100/          (10 portadoras 100×100 8bpp)
│   └── carriers_mixed/        (7 portadoras 300×300 + 1 portadora 200×200)
└── NN_<nombre>/               (un directorio por test)
    ├── README.md
    └── run.sh
```

## Cobertura

| Tests       | Categoría                                              |
|-------------|--------------------------------------------------------|
| 01–10       | Validación de CLI: parámetros inválidos / faltantes    |
| 20–28       | Round-trip distribuir → recuperar para k = 2..10       |
| 30–32       | Recuperación con subconjuntos de sombras y defaults    |
| 40–44       | Errores: portadoras insuficientes, bpp inválido,       |
|             | dimensiones incompatibles, formato de header           |

Total: **27 tests**.

## Ejecución

Generar fixtures (requiere Python 3 con Pillow):

```bash
python3 setup_fixtures.py
```

Compilar el binario:

```bash
make -C ../..              # produce ./sss
make -C ../.. compare_bmp  # produce tests/compare_bmp
```

Correr todos los tests:

```bash
bash run_all.sh
```

Correr un test individual:

```bash
bash 26_roundtrip_k8_n8/run.sh
```

Correr un subconjunto:

```bash
bash run_all.sh 01_cli_no_args 26_roundtrip_k8_n8
```

## Salida esperada

Cada test imprime una serie de líneas `[ok]` o `[FAIL]` durante su ejecución
y termina con una línea `PASS: <nombre>` o `FAIL: <nombre>`. El runner maestro
muestra al final un resumen con el conteo de tests pasados/fallados.

## Validación manual

Cada subcarpeta tiene un `README.md` que documenta:

- **Entrada**: el comando exacto que se ejecuta y los archivos involucrados.
- **Salida esperada**: exit code, mensajes, archivos creados, propiedades a
  verificar.
- **Validación**: cómo el script comprueba que la salida esperada se cumple.

Para validar manualmente sin correr el script: leer el README del test, repetir
el comando, observar el resultado y compararlo con lo descripto.

## Regenerar la suite

Si se quieren agregar/modificar tests, editar `generate_tests.py` y correr:

```bash
python3 generate_tests.py
```

Esto reescribe los `run.sh` y `README.md` de todos los tests preservando los
fixtures.
