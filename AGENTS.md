# CLAUDE.md

> **Sincronización obligatoria con `AGENTS.md`.** Los archivos `CLAUDE.md` y `AGENTS.md` deben mantenerse **idénticos en todo momento**. Si modificás uno, tenés que aplicar el mismo cambio en el otro en la misma operación — sin excepciones. Antes de dar por terminado cualquier cambio en alguno de los dos, verificá que el contenido coincida byte a byte.

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Status

Academic project (cryptography course). Currently only `docs/` exists — no source code yet. The reference material under `docs/` (especially `transcript_presentacion.txt` and `secret image sharing.pdf`) is the canonical spec; the implementation must conform to it.

## What this project implements

A combined **secret image sharing + steganography** system based on the paper *"Secret image sharing"* (Universidad China de Taipei). The program does two things:

1. **Distribute (`-d`)**: Take a secret BMP image and split it into `n` shadow images using a `(k, n)` threshold scheme (Shamir-style polynomial sharing). Each shadow is then hidden inside a carrier BMP via LSB steganography, producing `n` carrier images that look normal but encode the shadow.
2. **Recover (`-r`)**: Given `k` (or more) carrier images that contain hidden shadows, extract the shadows and reconstruct the original secret image using Lagrange interpolation.

## Hard constraints from the spec

- **Language: C or Java only.** Rust is explicitly disallowed (transcript line 7). The choice is driven by needing bit-level manipulation.
- **Image format: BMP only**, because pixels map directly to bytes (no compression). Work in grayscale.
- **Modular arithmetic in `GF(257)`**: all polynomial operations are `mod 257`. 257 is chosen over 251 so the full byte range `0..255` is representable. The trade-off: a polynomial evaluation can yield 256, which is unrepresentable in a byte — when that happens, perturb a coefficient (e.g., `50 → 51`) and recompute *all* shadow values for that polynomial until none equal 256. This makes recovery slightly lossy on perturbed pixels (acceptable per spec).
- **`k` range: `2 ≤ k ≤ 10`.**
- **Recovery uses Lagrange interpolation**, evaluated at `x = 0` using the "nested" form described in `Explicacion y ejemplo Shamir + Lagrange.pdf` and the transcript — work with numbers only (never carry `x` symbolically). The algorithm shrinks the polynomial degree by one each iteration: solve for `S_1` first, then substitute back to reduce to a degree-(k-2) problem, etc.
- **Permutation table**: a permutation step over the secret pixels uses a seed-driven RNG. The exact RNG implementation is specified in `docs/Tabla de Permutacion Implementacion.pdf` for both C and Java — **follow it exactly**, since the same seed must produce the same permutation across implementations to grade against the instructor's test files.
- **All multiplicative operations are modular**. Division = multiply by modular inverse mod 257. Precompute an inverse table.

## CLI shape

```
program {-d | -r} -secret <image.bmp> -k <number> [-n <number>] [-dir <directory>]
```

- `-d` distribute, `-r` recover
- `k` = minimum shadows needed to recover
- `n` = total shadows produced (distribute only); defaults per spec
- `dir` = directory holding carrier images (input for `-d`, input for `-r`)

## BMP handling rules

- **Do not assume pixel data starts at byte 54.** Read the offset from the BMP header — there can be padding before the pixel matrix.
- The header is preserved untouched in carriers *except* for these reserved bytes that the spec repurposes:
  - **Bytes 6–7**: store the permutation seed.
  - **Bytes 8–9**: store the shadow number (1..n) — i.e., which `x` value this carrier corresponds to in the polynomial. E.g., shadow 1 → `00 01`, shadow 13 → `0D 00` (watch endianness against the spec).
- For `k = 8`: each secret pixel produces 1 bit of shadow per carrier byte → carrier can be the same dimensions as the secret. Recovery then reuses the carrier's BMP header for the reconstructed secret image (since dimensions match).
- For `k ≠ 8`: shadow size is `M/k` of the secret. Decide whether to use a smaller carrier or pack >1 bit per byte (e.g., 4 bits in the LSB nibble). This is an open implementation decision the spec leaves to the implementer — document the choice in the deliverable analysis.

## Distribution algorithm (sketch)

1. Read secret BMP, locate pixel matrix.
2. Apply permutation (seed-based) to pixel order.
3. Group pixels into chunks of `k`. Each chunk forms polynomial coefficients `a_0 + a_1·x + ... + a_{k-1}·x^{k-1}`.
4. Evaluate the polynomial at `x = 1, 2, ..., n` mod 257. If any evaluation equals 256, bump a coefficient and redo.
5. Each evaluation contributes one byte to one shadow image.
6. Embed each shadow's bits into a carrier's LSBs. Write the seed into header bytes 6–7 and the shadow index into bytes 8–9.

## Recovery algorithm (sketch)

1. Read `k` carriers from `-dir`. Pull seed from bytes 6–7 (any one carrier), shadow index from bytes 8–9 of each.
2. Extract shadow bytes from LSBs.
3. For each pixel position across the `k` shadows, run Lagrange at `x = 0` using the nested/iterative reduction form to recover the `k` original coefficients in order.
4. Inverse-permute using the seed.
5. Reuse a carrier's BMP header (when sizes match, i.e., `k = 8`) to write the recovered secret BMP.

## Reference docs (read these before implementing)

- `docs/secret image sharing.pdf` — primary paper to implement.
- `docs/AMM.284-287.3025.pdf` — Thien & Lin's earlier paper (mod 251), referenced throughout.
- `docs/Apunte sobre Secreto Compartido.pdf` — course notes on threshold secret sharing.
- `docs/Tabla de Permutacion Implementacion.pdf` — **mandatory** RNG/permutation implementation for both C and Java.
- `docs/Explicacion y ejemplo Shamir + Lagrange.pdf` — worked Lagrange-at-zero example.
- `docs/transcript_presentacion.txt` — class transcript with assignment details, edge cases, and the `mod 257` rationale.

## Deliverable expectations

Beyond the working program, the assignment expects analysis of: discrepancies/errors found in the paper, debatable choices, and improvements proposed. Implementation decisions not covered by the paper (e.g., how the shadow index is hidden, how `k ≠ 8` is handled) must be documented as part of the analysis.
