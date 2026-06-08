# Secret Image Sharing + Steganography — TP Cripto 2026

Implementación en C de un sistema combinado de **compartición secreta de imágenes** y **esteganografía LSB**, basado en el paper *"Secret image sharing"* de Wu & Lo (GF(257)).

---

## Compilación

```bash
make            # produce el ejecutable 'sss'
make debug      # produce 'sss_dbg' con logs de depuración
make clean      # elimina objetos y ejecutables
```

Probado con gcc ≥ 9, C99, sin warnings.

---

## Uso

```
./sss {-d | -r} -secret <imagen.bmp> -k <número> [-n <número>] [-dir <directorio>]
```

| Parámetro | Descripción |
|-----------|-------------|
| `-d` | Modo **distribución**: oculta el secreto en las portadoras del directorio |
| `-r` | Modo **recuperación**: extrae el secreto de las portadoras y lo guarda en `-secret` |
| `-secret <bmp>` | En `-d`: imagen a ocultar. En `-r`: destino de la imagen recuperada |
| `-k <n>` | Umbral mínimo de sombras para recuperar (rango válido: 2–10) |
| `-n <n>` | *(Solo `-d`)* Total de sombras a generar. Por defecto: número de BMPs en `-dir` |
| `-dir <dir>` | Directorio con portadoras. Por defecto: directorio actual |

### Ejemplos

```bash
# Distribuir en 8 portadoras con umbral k=4
./sss -d -secret secreto.bmp -k 4 -n 8 -dir ./portadoras

# Recuperar usando al menos 4 portadoras del mismo directorio
./sss -r -secret recuperado.bmp -k 4 -dir ./portadoras
```

---

## Requerimientos de portadoras (`k ≠ 8`)

La imagen secreta se divide en bloques de `k` píxeles. Cada bloque genera 1 byte de sombra por portadora. Para ocultarlo en los LSBs de la portadora, el esquema de bits varía según `k`:

| Rango de `k` | Bits LSB usados | Relación píxeles portadora / byte sombra |
|:---:|:---:|:---:|
| 2–3 | **LSB4** (4 bits) | 2 px portadora por byte sombra |
| 4–7 | **LSB2** (2 bits) | 4 px portadora por byte sombra |
| 8–10 | **LSB1** (1 bit) | 8 px portadora por byte sombra |

> **Regla general:** cada portadora debe tener **al menos la misma cantidad de píxeles que la imagen secreta** (`ancho × alto`). Se recomienda que portadoras y secreto tengan las mismas dimensiones.

El programa valida esto al distribuir e informa si una portadora es insuficiente.

---

## Formato de imagen

- Sólo se soportan BMPs de **8 bits por píxel** (escala de grises, 1 byte = 1 píxel).
- El offset de píxeles se lee dinámicamente desde el header (no se asume byte 54).

## Metadatos en el header BMP

El programa reutiliza los bytes reservados del header para almacenar:

| Bytes | Contenido |
|-------|-----------|
| 6–7 | Semilla del LCG de 48 bits (little-endian `uint16`) |
| 8–9 | Número de sombra `x ∈ [1,n]` (little-endian `uint16`) |

---

## Tests

```bash
make test_math_permutation && ./tests/test_math_permutation  # GF(257), LCG, XOR
make test_distribute       && ./tests/test_distribute        # Horner, LSB, Lagrange
make test_bmp              && ./tests/test_bmp <in.bmp> <out.bmp>
```
