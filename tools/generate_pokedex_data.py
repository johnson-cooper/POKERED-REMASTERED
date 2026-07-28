#!/usr/bin/env python3
"""Generate indexed Pokédex metadata and descriptions from pokered."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = ROOT / "refs" / "pokered"
DATA = ROOT / "src" / "data"


def source_names() -> list[str]:
    text = (REFERENCE / "data" / "pokemon" / "base_stats.asm").read_text(encoding="utf-8", errors="replace")
    names = re.findall(r'INCLUDE "data/pokemon/base_stats/([^".]+)\.asm"', text)
    if len(names) != 150:
        raise RuntimeError(f"expected 150 species before Mew, got {len(names)}")
    names.append("mew")
    return names


def pascal_name(name: str) -> str:
    aliases = {"mrmime": "MrMime", "nidoranf": "NidoranF", "nidoranm": "NidoranM"}
    if name in aliases:
        return aliases[name]
    return "".join(part.capitalize() for part in name.split("-"))


def display_name(name: str) -> str:
    aliases = {
        "nidoranf": "NIDORAN F",
        "nidoranm": "NIDORAN M",
        "farfetchd": "FARFETCH'D",
        "mrmime": "MR.MIME",
    }
    return aliases.get(name, name.upper())


def enum_name(name: str) -> str:
    aliases = {
        "nidoranf": "MON_NIDORAN_F",
        "nidoranm": "MON_NIDORAN_M",
        "mrmime": "MON_MR_MIME",
    }
    return aliases.get(name, "MON_" + name.upper())


def parse_metadata() -> dict[str, tuple[str, int, int, int]]:
    text = (REFERENCE / "data" / "pokemon" / "dex_entries.asm").read_text(encoding="utf-8", errors="replace")
    pattern = re.compile(
        r"(?m)^(\w+DexEntry):\s*\n"
        r'\s*db "([^"]+)@"\s*\n'
        r"\s*db (\d+),(\d+)\s*\n"
        r"\s*dw (\d+)"
    )
    entries = {}
    for label, category, feet, inches, weight in pattern.findall(text):
        entries[label] = (category, int(feet), int(inches), int(weight))
    return entries


def parse_descriptions() -> dict[str, list[str]]:
    text = (REFERENCE / "data" / "pokemon" / "dex_text.asm").read_text(encoding="utf-8", errors="replace")
    blocks = re.split(r"(?m)^_(\w+DexEntry)::\s*$", text)
    descriptions: dict[str, list[str]] = {}
    for i in range(1, len(blocks), 2):
        label = blocks[i]
        block = blocks[i + 1].split("\n", 1)[1] if "\n" in blocks[i + 1] else blocks[i + 1]
        lines = []
        for directive, value in re.findall(r'(?m)^\s*(text|next|page|para) "([^"]*)"', block):
            value = value.upper().replace("#MON", "POKeMON")
            lines.append(value)
        if len(lines) >= 6:
            descriptions[label] = lines[:6]
    return descriptions


def c_string(value: str) -> str:
    return '"' + value.replace("\\", "\\\\").replace('"', '\\"') + '"'


def main() -> None:
    names = source_names()
    metadata = parse_metadata()
    descriptions = parse_descriptions()
    lines = ["#include \"pokedex.h\"", "", "const PokedexEntry g_pokedex_entries[NUM_POKEMON + 1] = {", "    [MON_NONE] = {0},"]

    for dex_number, name in enumerate(names, 1):
        label = pascal_name(name) + "DexEntry"
        if label not in metadata or label not in descriptions:
            raise RuntimeError(f"missing Pokédex data for {name}: {label}")
        category, feet, inches, weight = metadata[label]
        text = descriptions[label]
        height_text = f"HT {feet}'{inches}"
        weight_text = f"WT {weight / 10:.1f}LB"
        number_text = f"NO. {dex_number:03d}"
        lines.append(f"    [{enum_name(name)}] = {{")
        lines.append(f"        {c_string(display_name(name))}, {c_string(category)},")
        lines.append(f"        {c_string(height_text)}, {c_string(weight_text)},")
        lines.append(f"        {c_string(number_text)},")
        lines.append("        { " + ", ".join(c_string(line) for line in text[:3]) + " },")
        lines.append("        { " + ", ".join(c_string(line) for line in text[3:6]) + " },")
        lines.append("    },")

    lines += ["};", ""]
    with (DATA / "pokedex_entries.c").open("w", encoding="utf-8", newline="\n") as output:
        output.write("\n".join(lines))
    print(f"generated Pokédex entries for {len(names)} species")


if __name__ == "__main__":
    main()
