#!/usr/bin/env python3
"""
Convert a pokered grayscale PNG to a GBA 4bpp tiled u32 array.

Usage:
    python tools/convert_battle_sprite.py front  refs/pokered/gfx/pokemon/front/NAME.png  NAME_front
    python tools/convert_battle_sprite.py back   refs/pokered/gfx/pokemon/back/NAMEb.png  NAME_back

The output is a C array literal (512 u32 values) ready to paste into
src/data/gfx_battle_sprites.c.

GB grayscale -> GBA palette index mapping:
    255 (white)      -> 0  (transparent)
    170 (light gray) -> 2  (highlight)
     85 (dark gray)  -> 3  (body)
      0 (black)      -> 1  (outline)

Canvas: 64x64 pixels (8x8 tile grid, 8 u32 per tile = 512 u32 total).
Front sprites (40x40) are centered at offset (12, 12).
Back sprites  (32x32) are centered at offset (16, 16).
"""
from __future__ import annotations
import sys
from pathlib import Path
from PIL import Image

CANVAS = 64
TILE   = 8
WORDS  = (CANVAS * CANVAS) // TILE  # 512

GRAY_TO_IDX = {255: 0, 170: 2, 85: 3, 0: 1}


def png_to_gba(path: Path) -> list[int]:
    img = Image.open(path).convert("L")
    w, h = img.size
    pixels = list(img.tobytes())

    canvas = [0] * (CANVAS * CANVAS)
    ox = (CANVAS - w) // 2
    oy = (CANVAS - h) // 2
    for y in range(h):
        for x in range(w):
            idx = GRAY_TO_IDX.get(pixels[y * w + x], 0)
            canvas[(oy + y) * CANVAS + (ox + x)] = idx

    words: list[int] = []
    tiles_x = CANVAS // TILE
    tiles_y = CANVAS // TILE
    for ty in range(tiles_y):
        for tx in range(tiles_x):
            for row in range(TILE):
                val = 0
                for col in range(TILE):
                    px = canvas[(ty * TILE + row) * CANVAS + tx * TILE + col]
                    val |= (px & 0xF) << (col * 4)
                words.append(val)
    return words


def emit(words: list[int], array_name: str) -> str:
    lines = [f"const u32 {array_name}[BATTLE_WORDS] = {{"]
    for i in range(0, len(words), 8):
        chunk = words[i:i + 8]
        lines.append("    " + ", ".join(f"0x{v:08X}" for v in chunk) + ",")
    lines.append("};")
    return "\n".join(lines)


if __name__ == "__main__":
    if len(sys.argv) != 4:
        sys.exit("usage: convert_battle_sprite.py front|back <png> <array_name>")
    _, _, png_path, array_name = sys.argv
    words = png_to_gba(Path(png_path))
    print(emit(words, array_name))
