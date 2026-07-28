#!/usr/bin/env python3
"""Generate GBA sprite tables for all Gen 1 Pokémon from pokered PNGs."""
from __future__ import annotations

import re
from pathlib import Path
import sys

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = ROOT / "refs" / "pokered"
DATA = ROOT / "src" / "data"

BATTLE_CANVAS = 64
POKEDEX_CANVAS = 40


def species_files() -> list[str]:
    text = (REFERENCE / "data" / "pokemon" / "base_stats.asm").read_text()
    names = re.findall(r'INCLUDE "data/pokemon/base_stats/([^".]+)\.asm"', text)
    if len(names) != 150:
        raise RuntimeError(f"expected 150 base-stat includes before Mew, got {len(names)}")
    names.append("mew")
    return names


def pack_sprite(path: Path, canvas_size: int, *, crop: bool) -> list[int]:
    image = Image.open(path).convert("L")
    width, height = image.size
    if crop and (width > canvas_size or height > canvas_size):
        left = (width - canvas_size) // 2
        top = (height - canvas_size) // 2
        image = image.crop((left, top, left + canvas_size, top + canvas_size))
        width, height = image.size

    background = 2 if crop else 0
    pixels = [background] * (canvas_size * canvas_size)
    offset_x = (canvas_size - width) // 2
    offset_y = (canvas_size - height) // 2
    for y in range(height):
        for x in range(width):
            value = image.getpixel((x, y))
            if crop:
                mapped = {255: 2, 170: 4, 85: 3, 0: 1}.get(value, 2)
            else:
                mapped = {255: 0, 170: 2, 85: 3, 0: 1}.get(value, 0)
            pixels[(offset_y + y) * canvas_size + offset_x + x] = mapped

    words: list[int] = []
    for tile_y in range(canvas_size // 8):
        for tile_x in range(canvas_size // 8):
            for row in range(8):
                word = 0
                for col in range(8):
                    pixel = pixels[(tile_y * 8 + row) * canvas_size + tile_x * 8 + col]
                    word |= (pixel & 0xF) << (col * 4)
                words.append(word)
    return words


def c_name(name: str) -> str:
    aliases = {
        "nidoranf": "nidoran_f",
        "nidoranm": "nidoran_m",
    }
    name = aliases.get(name, name)
    return name.replace(".", "_").replace("-", "_")


def write_generated(path: Path, lines: list[str]) -> None:
    with path.open("w", encoding="utf-8", newline="\n") as output:
        output.write("\n".join(lines))


def image_name(name: str) -> str:
    return "mr.mime" if name == "mrmime" else name


def emit_array(lines: list[str], symbol: str, values: list[int], count: int) -> None:
    lines.append(f"const u32 {symbol}[{count}] = {{")
    for i in range(0, len(values), 8):
        lines.append("    " + ", ".join(f"0x{value:08X}" for value in values[i:i + 8]) + ",")
    lines.append("};")
    lines.append("")


def generate_battle(names: list[str]) -> None:
    lines = ["#include \"gfx_battle_sprites.h\"", "", "#define BATTLE_WORDS BATTLE_SPRITE_WORDS", ""]
    for name in names:
        slug = c_name(name)
        source_name = image_name(name)
        front = REFERENCE / "gfx" / "pokemon" / "front" / f"{source_name}.png"
        back = REFERENCE / "gfx" / "pokemon" / "back" / f"{source_name}b.png"
        if not front.exists() or not back.exists():
            raise RuntimeError(f"missing battle sprite for {name}: {front} / {back}")
        emit_array(lines, f"g_battle_{slug}_front", pack_sprite(front, BATTLE_CANVAS, crop=False), 512)
        emit_array(lines, f"g_battle_{slug}_back", pack_sprite(back, BATTLE_CANVAS, crop=False), 512)

    lines += ["static const u32 *const s_front_sprites[NUM_POKEMON + 1] = {", "    [MON_NONE] = g_battle_bulbasaur_front,"]
    for index, name in enumerate(names, 1):
        lines.append(f"    [{index}] = g_battle_{c_name(name)}_front,")
    lines += ["};", "", "static const u32 *const s_back_sprites[NUM_POKEMON + 1] = {", "    [MON_NONE] = g_battle_bulbasaur_back,"]
    for index, name in enumerate(names, 1):
        lines.append(f"    [{index}] = g_battle_{c_name(name)}_back,")
    lines += ["};", "", "void battle_sprite_load_front(PokemonId species, u32 *dest) {", "    if (species > NUM_POKEMON) species = MON_NONE;", "    const u32 *source = s_front_sprites[species];", "    for (u32 i = 0; i < BATTLE_SPRITE_WORDS; i++) dest[i] = source[i];", "}", "", "void battle_sprite_load_back(PokemonId species, u32 *dest) {", "    if (species > NUM_POKEMON) species = MON_NONE;", "    const u32 *source = s_back_sprites[species];", "    for (u32 i = 0; i < BATTLE_SPRITE_WORDS; i++) dest[i] = source[i];", "}", ""]
    write_generated(DATA / "gfx_battle_sprites.c", lines)


def generate_pokedex(names: list[str]) -> None:
    lines = ["#include \"gfx_pokedex.h\"", "#include \"pokemon.h\"", ""]
    for name in names:
        slug = c_name(name)
        front = REFERENCE / "gfx" / "pokemon" / "front" / f"{image_name(name)}.png"
        emit_array(lines, f"g_pokedex_{slug}_tiles", pack_sprite(front, POKEDEX_CANVAS, crop=True), 200)
    lines += ["static const u32 *const s_pokedex_sprites[NUM_POKEMON + 1] = {", "    [MON_NONE] = g_pokedex_bulbasaur_tiles,"]
    for index, name in enumerate(names, 1):
        lines.append(f"    [{index}] = g_pokedex_{c_name(name)}_tiles,")
    lines += ["};", "", "const u32 *pokedex_sprite_tiles(PokemonId species) {", "    if (species > NUM_POKEMON) species = MON_NONE;", "    return s_pokedex_sprites[species];", "}", ""]
    write_generated(DATA / "gfx_pokedex.c", lines)


def generate_headers(names: list[str]) -> None:
    battle = ["#pragma once", "", '#include \"types.h\"', '#include \"pokemon.h\"', "", "#define BATTLE_SPRITE_TILES 64", "#define BATTLE_SPRITE_WORDS (BATTLE_SPRITE_TILES * 8)", "", "void battle_sprite_load_front(PokemonId species, u32 *dest);", "void battle_sprite_load_back(PokemonId species, u32 *dest);", ""]
    write_generated(DATA / "gfx_battle_sprites.h", battle)
    pokedex = ["#pragma once", "", '#include \"types.h\"', '#include \"pokemon.h\"', ""]
    for name in names:
        pokedex.append(f"extern const u32 g_pokedex_{c_name(name)}_tiles[200];")
    pokedex += ["", "const u32 *pokedex_sprite_tiles(PokemonId species);", ""]
    write_generated(DATA / "gfx_pokedex.h", pokedex)


def main() -> None:
    names = species_files()
    generate_battle(names)
    generate_pokedex(names)
    generate_headers(names)
    print(f"generated sprite tables for {len(names)} species")


if __name__ == "__main__":
    main()
