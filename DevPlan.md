# Plan de Desarrollo Detallado: TP Secreto Compartido + Esteganografía (2026)

Este plan desglosa cada fase en tareas específicas para C o Java, alineadas estrictamente con el enunciado del TP y las notas de clase.

## Fase 1: Setup, CLI y Manejo Estricto de BMPs (Semana 1)
**Objetivo:** Tener el esqueleto del programa y dominar el formato de imagen antes de meterse con la matemática.

### Tareas:
- [x] **Módulo CLI (Command Line Interface):**
  - [x] Implementar parseo estricto de parámetros, respetando mayúsculas y minúsculas.
  - [x] Soportar parámetros requeridos: `-d` (distribuir) o `-r` (recuperar), `-secret <imagen.bmp>`, `-k <numero>` (entre 2 y 10).
  - [x] Soportar parámetros opcionales: `-n <numero>` (mínimo 2), `-dir <directorio>`. 
  - [x] Lógica por defecto: Si no se usa `-dir`, buscar en el directorio actual. Si no se usa `-n`, inferir la cantidad leyendo las imágenes del directorio.
  - [x] Manejo de errores: Si hay un error de sintaxis, explicitar el error e informar la sintaxis correcta al usuario.

- [x] **Módulo BMP (Lectura/Escritura):**
  - [x] Validar que se trabaje estrictamente con imágenes BMP de 8 bits por píxel (escala de grises, 1 byte = 1 píxel).
  - [x] **Offset dinámico:** Leer el offset en el encabezado de 54 bytes para saber dónde empieza realmente la matriz de píxeles (no asumir que siempre es el byte 54).
  - [x] Controlar Endianness: Tener cuidado con la codificación Little/Big Endian al leer los bytes del encabezado.
  - [x] Lectura correcta: Procesar la matriz de píxeles (se lee de abajo hacia arriba y de izquierda a derecha).

---

## Fase 2: Permutación y Matemática en GF(257) (Semana 2)
**Objetivo:** Construir el "motor" criptográfico del sistema.

### Tareas:
- [x] **Módulo de Permutación:**
  - [x] Extraer la semilla (2 bytes) y ocultarla en los bytes reservados 6 y 7 del encabezado de la imagen portadora.
  - [x] Implementar *exactamente* el algoritmo de generación pseudoaleatoria provisto en la documentación (`Tabla de Permutacion Implementacion.pdf`). 
    - *En C:* Usar `setSeed(int64_t)` y `nextChar(void)` con variables de 48 bits.
    - *En Java:* Instanciar `java.util.Random` con `setSeed` and `nextInt(256)`.
  - [x] Permutar todos los píxeles de la imagen secreta antes de dividirlos.

- [x] **Módulo Matemático GF(257):**
  - [x] Implementar aritmética modular en $Z_{257}$ (suma, resta, multiplicación).
  - [x] Implementar división modular calculando el inverso multiplicativo módulo 257. (Recomendación: precomputar una tabla estática `[257]` en el código para acelerar cálculos).

---

## Fase 3: Distribución (Shamir) y Esteganografía LSB (Semana 2-3)
**Objetivo:** Conectar la matemática y las imágenes para ocultar el secreto en portadoras.

### Tareas:
- [x] **Generación de Polinomios (Distribución):**
  - [x] Agrupar los píxeles permutados de a $k$ para formar polinomios de grado $k-1$.
  - [x] Asignar esos $k$ píxeles a **todos** los coeficientes del polinomio ($a_0, a_1, \dots, a_{k-1}$). No hay coeficientes aleatorios. Esto logra la compresión de sombras del paper de Thien & Lin: cada $k$ píxeles de secreto generan **1 solo byte** de sombra por evaluación.
  - [x] Evaluar el polinomio en $x=1, 2, ..., n$ para generar el byte (sombra) correspondiente a cada portadora.
  - [x] **Regla estricta del 256:** Si alguna evaluación arroja exactamente 256, alterar un coeficiente del polinomio (ej. $a_0 = (a_0 + 1) \bmod 256$) y recalcular todo el polinomio para TODAS las sombras hasta que ningún valor sea 256. Esto genera el ruido (lossy) esperado en la recuperación.

- [x] **Embebido Esteganográfico (LSB Replacement):**
  - [x] Guardar el "número de sombra" ($x$ usado en el polinomio, ej. 1, 2... n) en los bytes reservados 8 y 9 de cada portadora.
  - [x] Ocultar los bytes de sombra generados particionándolos bit a bit y reemplazando el bit menos significativo (LSB) de los bytes de la matriz de píxeles de la portadora (comenzando desde el pixel offset).

- [x] **Decisión Arquitectónica sobre Portadoras (Documentar):**
  - [x] *Si $k=8$:* Cada 8 píxeles de secreto → 1 byte de sombra. Para ocultarlo con LSB1 se necesitan 8 bytes de portadora. Resultado: portadoras del mismo tamaño que la imagen secreta.
  - [x] *Si $k \neq 8$:* La compresión es de $k$ a 1. Documentar en el informe cómo se manejan las portadoras y/o si se cambia a LSB2, LSB4, etc., para compensar la diferencia de tamaño.

---

## Fase 4: Recuperación mediante Lagrange Reducido (Semana 3)
**Objetivo:** Implementar la extracción y reconstrucción eficiente de la imagen secreta.

### Tareas:
- [x] **Extracción de Metadata y Sombras:**
  - [x] Leer los bytes 6 y 7 de cualquier portadora para obtener la semilla de permutación.
  - [x] Leer los bytes 8 y 9 de cada portadora para saber qué "sombra" ($x$) representa.
  - [x] Extraer los LSB del cuerpo del archivo BMP para recuperar los bytes de las sombras.

- [x] **Resolución de Sistemas de Ecuaciones (Lagrange):**
  - [x] Implementar el método de Lagrange reducido, evaluado directamente en $x=0$.
  - [x] Trabajar numéricamente (sin llevar $x$ de forma simbólica).
  - [x] Algoritmo "encajado" (reducción iterativa): Encontrar $S_1$ (término independiente), restar ese valor a las sombras, dividir por $x$ (multiplicando por el inverso modular) para reducir el grado del polinomio, y repetir hasta obtener los $k$ coeficientes.

- [x] **Reconstrucción:**
  - [x] Aplicar la permutación inversa (usando la semilla extraída) para reordenar los píxeles al formato original de la imagen secreta.
  - [x] Si $k=8$, clonar el encabezado de cualquiera de las imágenes portadoras para el archivo `.bmp` secreto recuperado.

---

## Fase 5: Testing en Pampero e Informe Final (Semana 4)
**Objetivo:** Validar requisitos de entrega y justificar las decisiones académicas.

### Tareas:
- [ ] **Pruebas en Servidor (Pampero ITBA):**
  - [ ] Clonar el código en el entorno `pampero`.
  - [ ] Verificar que compila sin errores ni warnings utilizando `gcc` / `javac` del servidor.
  - [ ] Ejecutar pruebas de recuperación (-r) con los archivos provistos por la cátedra. (Condición estricta de aprobación).

- [ ] **Redacción del Informe (40% de la nota final):**
  - [ ] Discutir la organización formal, claridad y notación del paper de Luang-Shyr Wu y Tsung-Ming.
  - [ ] Explicar detalladamente por qué la imagen recuperada **no** es exactamente igual a la original (justificar la pérdida de información originada por la regla de mitigación del 256).
  - [ ] Explicar y justificar el criterio utilizado para el tamaño de las portadoras y el embebido cuando $k \neq 8$.
  - [ ] Analizar la seguridad de almacenar la semilla (bytes 6-7) y el número de sombra (bytes 8-9) en la cabecera del BMP y proponer alternativas.
  - [ ] Discutir la facilidad de implementación, escalabilidad a imágenes de 24 bits (color) y posibles mejoras al algoritmo.