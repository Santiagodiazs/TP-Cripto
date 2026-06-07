# Cuestiones pendientes — TP Cripto

Archivo vivo. Se va actualizando conforme se descubren cosas. Las casillas marcadas (`[x]`) son cosas verificadas/cerradas; las vacías (`[ ]`) son lo que falta.

---

## 🚨 Críticos para la entrega

- [ ] **Fecha de entrega: 8 de junio.** Hoy es 2026-06-07 → **1 día de margen**. Cualquier cosa que requiera trabajo serio (debuggear el diff alto, completar stubs del informe) debe priorizarse hoy.
- [ ] **Esperar archivos definitivos de la cátedra.** El home de `tpinausigcastillo@pampero` no tiene material del TP. Cuando los entreguen:
  1. Bajarlos local + subirlos a pampero (`rsync`).
  2. Correr `./sss -r -secret out.bmp -k <K> -dir <dir>` con cada valor de `k` posible (2..10) hasta dar con el correcto.
  3. Comparar la imagen recuperada con la imagen secreta que dé la cátedra (si entrega una).
- [ ] **Generar README.md de uso** (la consigna §4.2.5 lo pide explícitamente: *"Deberán indicar en el README si hay algún requerimiento especial de las portadoras para este caso [k≠8]"*). Debe incluir: comando, parámetros, requerimiento de portadoras del mismo tamaño que el secreto, esquema LSB1/2/4 según k.
- [ ] **Verificar que el binario final pasa en pampero con `make clean && make`.** Ya verificado en sesión actual (gcc 16.1.1, sin warnings), pero re-correrlo justo antes de entregar.

---

## 🐛 Bugs / código a revisar

- [x] **Diff máximo 143–195 en píxeles divergentes del round-trip.** **NO es un bug**, es comportamiento esperado. Análisis:
  - La perturbación en el espacio del polinomio está acotada por `m ≤ n` (a lo sumo `n` valores prohibidos para `m` en `[0, 256]`).
  - PERO: el XOR con la máscara LCG **no preserva la estructura aditiva**. Un cambio de `+m` (con `m ≤ 8`) en el coeficiente perturbado puede amplificarse hasta ±255 en el byte final del píxel, porque el carry de la suma puede llegar a un bit alto que la máscara invierte.
  - **Ejemplo:** `pixel=255, mask_byte=128, m=1` → `recovered = ((255⊕128) + 1) ⊕ 128 = 0`, diff = 255.
  - El recuento de bloques perturbados coincide con la predicción `n/257 ≈ 3.1%` (test: 3.12% para k=8, 3.22% para k=4).
  - **Conclusión:** documentar esta amplificación en el informe §2 como complemento a la regla del 256.

- [x] **Discrepancia con el paper Wu/Lo en la regla del 256.** Analizada — la divergencia es funcionalmente equivalente para el round-trip y la interoperabilidad con la cátedra (Lagrange en `-r` es agnóstico a qué coeficiente se perturbó en `-d`). El paper decrementa el primer no-cero; el código incrementa siempre `a_0`. Nuestra variante es más simple y siempre termina (no necesita la protección "first non-zero" porque incrementar 0 da 1, válido). **Decisión:** dejar el código como está. Documentar la divergencia en el informe §2 (ya está).

- [ ] **`sss.c:476-478` clampa silenciosamente valores ≥256 a 0 en la recuperación.** Comentario dice que no debería pasar nunca, pero si pasa enmascara un bug. Cambiar a `assert()` o `fprintf(stderr, ...)` para detectar el edge case.

- [ ] **`bmp_save` no invierte filas** (`src/bmp.c:157`, comentado intencionalmente). Si una portadora BMP es bottom-up (height negativa o sin flag explícito), los píxeles se escriben tal cual; funciona porque el header tampoco se toca. Pero el merge de header de portadora → secreto en recovery (`sss.c:486-498`) puede heredar un height firmado incorrecto. Verificar con BMPs bottom-up reales.

- [ ] **Confirmar endianness de bytes 8-9 contra el ejemplo de la consigna.** La consigna §4.2.5 muestra que sombra=3 se almacena como byte8=03, byte9=00. El código (`src/bmp.c:200-202`) usa `write_u16le`, que para 3 → `[03, 00]`. **Coherente.** Pero `CLAUDE.md` menciona "shadow 13 → 0D 00" como ejemplo — confirmar que el código produce eso (lo hace: 13 = 0x000D → LE → `[0D, 00]`). ✅

---

## 🔬 Investigación / a confirmar

- [ ] **Confirmar que los archivos de `archivosdeprueba/` y `archivosG5/` son sombras pre-distribuidas y descubrir el `k` correspondiente.** Estado de la hipótesis: probable. Verificado parcialmente para `archivosdeprueba/Albertssd.bmp` (seed=0x59D9, shadow_num=1). Falta:
  1. Leer bytes 6–9 de los 8 archivos en cada carpeta y confirmar `seed` igual entre los 8 y `shadow_num` correlativos `1..8`. Un script de unas 10 líneas con `xxd` o un C de 30 líneas resuelve.
  2. Correr `./sss -r -secret out_kX.bmp -k X -dir archivosdeprueba/` para `X` en `{2,3,...,8}` y observar cuál salida es una imagen reconocible (no ruido). Ese es el `k`.
  3. Repetir para `archivosG5/` (es el set definitivo del grupo 5, probablemente con `k` distinto).
  - El recovery es agnóstico a qué `m` perturbó cada bloque: Lagrange reconstruye los coeficientes efectivos sin importar la regla de mitigación que se haya usado en distribución. Por lo tanto, la diferencia paper-vs-código (cuestión 3) no impide recuperar archivos de la cátedra.

- [ ] **Verificar que la máscara XOR del código es la del paper Wu/Lo, no la "permutación" de Thien-Lin.** ✅ **Resuelto:** revisando el paper Wu/Lo (paso 1: *"Take the XOR operation to a pre-defined random table R and O"*), la implementación actual (XOR byte-a-byte con stream del LCG) es exactamente lo prescripto. Thien-Lin sí usa reordenamiento Fisher-Yates, pero Wu/Lo (que es lo que pide la consigna) usa XOR.

---

## 📝 Informe — stubs por completar

Archivo: `informe/informe.tex`. Compila con `cd informe && make pdf`.

- [x] **§1 — Análisis del paper Wu/Lo** (consigna 5.1) — organización formal, descripción de algoritmos, notación (incluye crítica al typo del paso 5 con $a_{n-1}$ vs $a_{r-1}$).
- [x] **§5.a — Facilidad de implementación** (consigna 5.5.a) — partes triviales vs delicadas, comentario sobre testing y estimación de esfuerzo.
- [x] **§6 — Dificultades en la lectura del paper y/o implementación** (consigna 5.6) — agrupadas en lectura, implementación y testing.
- [x] **§8 — Casos de aplicación** (consigna 5.8) — siete escenarios + justificación de por qué $(k,n)$ + esteganografía es superior a cifrado+replicación.

---

## 🚀 Mejoras posibles (post-entrega)

No requeridas para aprobar, pero ya documentadas en `informe.tex` §7. Listadas acá por completitud:

- [ ] Versión lossless de Thien-Lin (sección 3.3 del paper original).
- [ ] Soporte 24bpp (separación por planos B, G, R).
- [ ] Cifrado de la semilla con clave maestra fuera de banda.
- [ ] Verificación de integridad por sombra (CRC32 / MAC).
- [ ] Verifiable Secret Sharing (Feldman/Pedersen).
- [ ] Distribución progresiva (Chen-Lin 2005).
- [ ] Endurecimiento esteganográfico: semilla en LSBs de píxeles fijos en vez de bytes 6-7.
- [ ] Tabla de inversos por Euclides extendido en O(p log p) en vez de fuerza bruta O(p²).

---

## ✅ Cerradas en esta sesión

- [x] **Lectura completa del código (`src/`), tests, Makefile, assets y docs.**
- [x] **Lectura de la consigna oficial** (`docs/Trabajo Practico de Implementacion2026_1.pdf`).
- [x] **Lectura del paper Wu/Lo, Thien-Lin y transcript** para fundamentar el informe.
- [x] **Compilación verificada en pampero** (gcc 16.1.1, sin warnings).
- [x] **Tests unitarios pasan en pampero** (math, LCG, LSB, Horner, Lagrange, mitigación 256).
- [x] **Round-trip end-to-end probado en pampero** (k=8 y k=4 con BMPs propios).
- [x] **Esqueleto LaTeX creado** (`informe/informe.tex`, 8 secciones, 4 completas + 4 stubs).
- [x] **Confirmada la equivalencia XOR-mask ↔ paper Wu/Lo** (no es bug, es lo que pide el paper).
- [x] **Informe LaTeX completo**: las 8 cuestiones de la sección 5 de la consigna están escritas; PDF 10 páginas, compila sin errores.
- [x] **Análisis del diff 143–195**: descartado como bug, es amplificación XOR de la perturbación del 256 (`m ≤ n` pero diff en píxel hasta 255).
- [x] **Análisis de la discrepancia con el paper en la regla del 256**: funcionalmente equivalente para round-trip y para `-r` sobre archivos de cátedra; decisión de dejar el código como está.
