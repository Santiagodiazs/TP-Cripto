# Cuestiones pendientes — TP Cripto

Archivo vivo. `[x]` = cerrado, `[ ]` = abierto.

Estado general: **muy cerca de entregar**. Lo crítico está hecho. Quedan ítems
de pulido y mejoras post-entrega.

---

## 🚨 Críticos para la entrega

- [ ] **Fecha de entrega: 8 de junio.** Hoy 2026-06-07 (mañana).
- [ ] **Última verificación en pampero** justo antes de entregar:
  - `git pull origin main` en pampero.
  - `make clean && make` debe compilar sin warnings con `gcc 16.1.1` (ya verificado).
  - `./sss -r -secret out.bmp -k 8 -dir archivosG5/` debe producir el BMP de
    136 678 bytes con la Torre Eiffel sin shear (ya verificado vía SSH).

---

## 📦 Recuperación con los sets provistos (todo verificado)

- [x] **`archivosdeprueba/` (300×300, sin padding)** → Torre Eiffel diurna,
  k=8, seed `0x59D9`.
- [x] **`archivosG5/` (450×300, con padding)** → Torre Eiffel diurna, k=8,
  seed `0x48E1`. Requirió el fix de padding en `bmp_save`.
- [x] **`grupootro/` (300×450, sin padding)** → Torre Eiffel nocturna con
  Trocadéro, k=8, seed `0x166F`.
- [x] **Round-trip en pampero**: `rsync` + `ssh` + `scp` con `sshpass -p '123'`.
  El BMP recuperado por pampero coincide byte-a-byte con el local (mismo gcc
  pipeline, distinto SO).

---

## 🐛 Código — abiertos

- [ ] **`sss.c:476-478` clampa silenciosamente valores ≥256 a 0 en la
  recuperación.** El comentario dice que no debería pasar nunca, pero si
  pasa enmascara un bug. Sugerido: cambiar a `assert()` o
  `fprintf(stderr, ...)` para diagnóstico. **Baja prioridad** (defensivo).

- [ ] **Filas bottom-up en BMP.** `bmp_load` toma `abs(height)` pero no
  invierte filas, y `bmp_save` tampoco. Para BMPs con altura negativa
  (top-down explícito) o bottom-up clásico, las dimensiones se preservan
  pero el orden de filas puede heredarse mal del header. No es un problema
  con los assets de la cátedra (todos top-down con altura positiva), pero
  con BMPs externos podría confundir. **Baja prioridad** (no aparece en la
  consigna).

## 🐛 Código — cerrados en esta sesión

- [x] **Diff máximo 143–195 en píxeles divergentes**: descartado como bug.
  Es la amplificación XOR de la perturbación de la regla del 256.
- [x] **Discrepancia con el paper Wu/Lo en la regla del 256**: documentada,
  funcionalmente equivalente. Código se deja como está.
- [x] **Padding de fila en `bmp_save`** (450×300 mostraba shear): resuelto
  con fix asimétrico — sólo `bmp_save` recorre el buffer en pasos de
  `width + row_pad`. `bmp_load` queda contiguo para preservar interop.
  Verificado: archivosG5 ahora muestra Eiffel limpia.
- [x] **Endianness bytes 8-9**: confirmada coincidencia con consigna §4.2.5.
- [x] **XOR mask vs permutación**: confirmada coincidencia con paper Wu/Lo
  paso 1 (no es bug).

---

## 📝 Informe — estado

- [x] **8 cuestiones de la consigna §5** completas, sin stubs.
- [x] **Sección "Recuperación del secreto provisto por la cátedra"** agregada
  (sin numerar, entre abstract y §1).
- [x] **Carátula con logo ITBA + roster del grupo** (Lucila, Santiago,
  Tomás, Katia).
- [x] **40 correcciones del review aplicadas** (correcciones.txt).
- [x] **Documentación del fix de padding** en §5.a y §6.
- [x] PDF 10 páginas, compila sin warnings.

---

## 🚀 Mejoras posibles (post-entrega)

Ya documentadas en `informe.tex` §7. Listadas para tracking:

- [ ] Versión lossless de Thien-Lin (sección 3.3 del paper original).
- [ ] Soporte 24bpp (separación por planos B, G, R).
- [ ] Cifrado de la semilla con clave maestra fuera de banda.
- [ ] Verificación de integridad por sombra (CRC32 / MAC).
- [ ] Endurecimiento esteganográfico (semilla en LSBs de píxeles fijos en vez
  de bytes 6-7).

---

## ✅ Hitos cerrados de la sesión completa

- Lectura completa del código, tests, Makefile, assets y todos los docs de
  `docs/`.
- Implementación verificada en pampero (gcc 16.1.1, sin warnings).
- Tests unitarios (math, LCG, LSB, Horner, Lagrange, mitigación) y tests e2e
  (27 casos, viven en la rama `testing`).
- Informe LaTeX completo (8 secciones + carátula + sección de recuperación
  cátedra + correcciones + documentación del fix de padding).
- Recuperación verificada en los tres sets entregados (`archivosdeprueba/`,
  `archivosG5/`, `grupootro/`) — todos Torre Eiffel, k=8 en los tres.
- Fix de padding asimétrico en `bmp_save` que destraba la visualización de
  archivosG5 sin romper interop con sombras de otras implementaciones.
- `README.md` de uso ya en el repo (commit de Santiago).
- Round-trip de prueba a pampero vía SSH + SFTP + SCP completado con
  resultado byte-exacto contra la ejecución local.
