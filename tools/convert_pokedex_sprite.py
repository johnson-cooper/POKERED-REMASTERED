#!/usr/bin/env python3
"""
Convert a pokered grayscale PNG to a GBA party/pokedex sprite C array.

The party status screen uses a 5x5 tile (40x40 pixel) 4bpp sprite.
Canvas size is always 40x40; larger sources are center-cropped.

Palette mapping (matches existing g_pokedex_*_tiles arrays):
  255 (white)      -> 2  (background fill)
  170 (light gray) -> 4  (light body shade)
   85 (dark gray)  -> 3  (dark body shade)
    0 (black)      -> 1  (outline)

Usage:
    python tools/convert_pokedex_sprite.py refs/.../front/NAME.png g_pokedex_NAME_tiles
"""
from __future__ import annotations
import sys
from pathlib import Path
from PIL import Image

CANVAS = 40
TILE   = 8
TILES  = CANVAS // TILE        # 5
WORDS  = TILES * TILES * TILE  # 200

GRAY_TO_IDX = {255: 2, 170: 4, 85: 3, 0: 1}


def png_to_gba(path: Path) -> list[int]:
    img = Image.open(path).convert("L")
    w, h = img.size

    # Center-crop if larger than canvas
    if w > CANVAS or h > CANVAS:
        ox = (w - CANVAS) // 2
        oy = (h - CANVAS) // 2
        img = img.crop((ox, oy, ox + CANVAS, oy + CANVAS))
        w, h = CANVAS, CANVAS

    pixels = list(img.tobytes())
    canvas = [2] * (CANVAS * CANVAS)  # background = index 2
    dx = (CANVAS - w) // 2
    dy = (CANVAS - h) // 2
    for y in range(h):
        for x in range(w):
            canvas[(dy + y) * CANVAS + (dx + x)] = GRAY_TO_IDX.get(pixels[y * w + x], 2)

    words: list[int] = []
    for ty in range(TILES):
        for tx in range(TILES):
            for row in range(TILE):
                val = 0
                for col in range(TILE):
                    px = canvas[(ty * TILE + row) * CANVAS + tx * TILE + col]
                    val |= (px & 0xF) << (col * 4)
                words.append(val)
    return words


def emit(words: list[int], array_name: str) -> str:
    chunks = [words[i:i+8] for i in range(0, len(words), 8)]
    body = ", ".join(f"0x{v:08X}" for chunk in chunks for v in chunk)
    return f"const u32 {array_name}[{WORDS}] = {{ {body} }};"


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit("usage: convert_pokedex_sprite.py <png> <array_name>")
    _, png_path, array_name = sys.argv
    words = png_to_gba(Path(png_path))
    print(emit(words, array_name))
