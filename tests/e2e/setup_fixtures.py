#!/usr/bin/env python3
"""
Genera las imágenes fixture para los tests e2e.

Produce BMPs de 8 bpp en escala de grises (modo "L" de PIL → BMP indexado de
256 colores con offset de píxel a 1078, exactamente como los assets de la
cátedra).

Idempotente: salta archivos que ya existen.
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import random
import sys

HERE = Path(__file__).resolve().parent
FIX = HERE / "fixtures"

def ensure_dir(p: Path):
    p.mkdir(parents=True, exist_ok=True)

def save_8bpp(img: Image.Image, path: Path):
    """Guarda como BMP de 8 bpp, modo 'L' (grayscale indexed)."""
    if path.exists():
        return
    img.convert("L").save(path, format="BMP")

def make_text(size, text, font_size):
    img = Image.new("L", size, color=255)
    draw = ImageDraw.Draw(img)
    try:
        font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial Bold.ttf", font_size)
    except OSError:
        font = ImageFont.load_default()
    bbox = draw.textbbox((0, 0), text, font=font)
    tw = bbox[2] - bbox[0]
    th = bbox[3] - bbox[1]
    x = (size[0] - tw) // 2 - bbox[0]
    y = (size[1] - th) // 2 - bbox[1]
    draw.text((x, y), text, fill=0, font=font)
    return img

def make_noise(size, seed):
    rng = random.Random(seed)
    img = Image.new("L", size)
    pixels = bytes(rng.randint(0, 255) for _ in range(size[0] * size[1]))
    img.frombytes(pixels)
    return img

def make_gradient(size, seed):
    """Gradiente diagonal con offset según seed para diferenciar."""
    img = Image.new("L", size)
    w, h = size
    px = img.load()
    for y in range(h):
        for x in range(w):
            v = ((x + y + seed * 17) * 255) // (w + h)
            px[x, y] = v % 256
    return img

def main():
    ensure_dir(FIX)
    ensure_dir(FIX / "carriers_300")
    ensure_dir(FIX / "carriers_100")
    ensure_dir(FIX / "carriers_mixed")

    # Imagen secreta 300x300 con texto "SECRETO"
    save_8bpp(make_text((300, 300), "SECRETO", 60), FIX / "secret_300.bmp")

    # Imagen secreta 100x100 con texto "TP"
    save_8bpp(make_text((100, 100), "TP", 30), FIX / "secret_100.bmp")

    # 10 portadoras 300x300, cada una con patrón distinto (noise determinista)
    for i in range(1, 11):
        save_8bpp(make_noise((300, 300), seed=i),
                  FIX / "carriers_300" / f"carrier_{i:02d}.bmp")

    # 10 portadoras 100x100
    for i in range(1, 11):
        save_8bpp(make_noise((100, 100), seed=100 + i),
                  FIX / "carriers_100" / f"carrier_{i:02d}.bmp")

    # Mixed: 7 portadoras 300x300 + 1 de 200x200 (para test de mismatch)
    for i in range(1, 8):
        src = FIX / "carriers_300" / f"carrier_{i:02d}.bmp"
        dst = FIX / "carriers_mixed" / f"carrier_{i:02d}.bmp"
        if not dst.exists():
            dst.write_bytes(src.read_bytes())
    save_8bpp(make_noise((200, 200), seed=999),
              FIX / "carriers_mixed" / "carrier_08.bmp")

    # Imagen 24bpp para test de formato inválido (PIL "RGB" → BMP 24bpp)
    p24 = FIX / "secret_24bpp.bmp"
    if not p24.exists():
        Image.new("RGB", (100, 100), color=(128, 64, 32)).save(p24, format="BMP")

    # Archivo que no es BMP
    not_bmp = FIX / "not_a_bmp.txt"
    if not not_bmp.exists():
        not_bmp.write_text("Esto no es un archivo BMP.\n")

    # Resumen
    print(f"Fixtures generadas en {FIX}:")
    for p in sorted(FIX.iterdir()):
        if p.is_file():
            print(f"  {p.name}  ({p.stat().st_size} bytes)")
        else:
            count = len(list(p.iterdir()))
            print(f"  {p.name}/  ({count} archivos)")

if __name__ == "__main__":
    main()
