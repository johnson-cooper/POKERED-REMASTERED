#!/usr/bin/env python3
"""Generate complete Gen 1 level-up learnsets and evolution records."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = ROOT / "refs" / "pokered"


def source_names() -> list[str]:
    text = (REFERENCE / "data" / "pokemon" / "base_stats.asm").read_text(encoding="utf-8", errors="replace")
    names = re.findall(r'INCLUDE "data/pokemon/base_stats/([^".]+)\.asm"', text)
    if len(names) != 150:
        raise RuntimeError(f"expected 150 species before Mew, got {len(names)}")
    return names + ["mew"]


def enum_name(name: str) -> str:
    return {
        "nidoranf": "MON_NIDORAN_F",
        "nidoranm": "MON_NIDORAN_M",
        "mrmime": "MON_MR_MIME",
    }.get(name, "MON_" + name.upper())


def move_name(name: str) -> str:
    return {"PSYCHIC_M": "MOVE_PSYCHIC"}.get(name.upper(), "MOVE_" + name.upper())


def parse_blocks() -> dict[str, tuple[list[tuple[str, list[str]]], list[tuple[int, str]]]]:
    text = (REFERENCE / "data" / "pokemon" / "evos_moves.asm").read_text(encoding="utf-8", errors="replace")
    chunks = re.split(r"(?m)^([A-Za-z0-9]+EvosMoves):\s*$", text)
    blocks = {}
    for i in range(1, len(chunks), 2):
        name, block = chunks[i], chunks[i + 1]
        evolution_text, learnset_text = (block.split("; Learnset", 1) + [""])[:2]
        evolutions = []
        for line in evolution_text.splitlines():
            line = line.split(";", 1)[0]
            values = [v.strip() for v in line.split(",")]
            if not values or "EVOLVE_" not in values[0]:
                continue
            method = values[0].split()[-1]
            if method == "EVOLVE_LEVEL" and len(values) == 3:
                evolutions.append((method, values[1:]))
            elif method == "EVOLVE_ITEM" and len(values) == 4:
                evolutions.append((method, values[1:]))
            elif method == "EVOLVE_TRADE" and len(values) == 3:
                evolutions.append((method, values[1:]))
        learnset = []
        for line in learnset_text.splitlines():
            match = re.match(r"\s*db\s+(\d+)\s*,\s*([A-Za-z0-9_]+)", line)
            if match:
                learnset.append((int(match.group(1)), match.group(2)))
        blocks[name.removesuffix("EvosMoves").lower()] = (evolutions, learnset)
    return blocks


def main() -> None:
    names = source_names()
    blocks = parse_blocks()
    item_names = {"FIRE_STONE", "THUNDER_STONE", "WATER_STONE", "LEAF_STONE", "MOON_STONE"}
    max_moves = max(len(blocks[name][1]) for name in names)
    max_evos = max(len(blocks[name][0]) for name in names)
    standard_trade_level = 36
    if max_moves + 1 > 10:
        raise RuntimeError(f"MAX_LEARNSET_MOVES must support {max_moves} moves plus sentinel")
    if max_evos > 3:
        raise RuntimeError(f"MAX_EVOLUTIONS must support {max_evos} entries")

    learnset_lines = ['#include "learnsets.h"', "", "const PokemonLearnset g_learnsets[NUM_POKEMON + 1] = {", "    [MON_NONE] = {{0}},"]
    evolution_lines = ['#include "evolution.h"', "", "const PokemonEvolution g_pokemon_evolutions[NUM_POKEMON + 1] = {"]
    for name in names:
        evolutions, learnset = blocks[name]
        mon = enum_name(name)
        learnset_lines.append(f"    [{mon}] = {{")
        for level, move in learnset:
            learnset_lines.append(f"        {{ {level:2d}, {move_name(move)} }},")
        learnset_lines.append("        { 0, MOVE_NONE },")
        learnset_lines.append("    },")

        evolution_lines.append(f"    [{mon}] = {{")
        for method, values in evolutions:
            if method == "EVOLVE_LEVEL":
                evolution_lines.append(f"        {{ EVOLUTION_LEVEL, {values[0]}, EVO_ITEM_NONE, {enum_name(values[1].lower())} }},")
            elif method == "EVOLVE_ITEM":
                item = values[0].upper()
                if item not in item_names:
                    raise RuntimeError(f"unsupported evolution item {item}")
                evolution_lines.append(f"        {{ EVOLUTION_ITEM, {values[1]}, EVO_ITEM_{item}, {enum_name(values[2].lower())} }},")
            elif method == "EVOLVE_TRADE":
                # This project does not use link trades. Convert Gen 1 trade
                # evolutions into ordinary level evolutions at level 36.
                evolution_lines.append(f"        {{ EVOLUTION_LEVEL, {standard_trade_level}, EVO_ITEM_NONE, {enum_name(values[1].lower())} }},")
        evolution_lines.append("        { EVOLUTION_NONE, 0, EVO_ITEM_NONE, MON_NONE },")
        evolution_lines.append("    },")
    learnset_lines += ["};", "", "MoveId learnset_move_at_level(PokemonId species, u8 level) {", "    if (species == MON_NONE || species > NUM_POKEMON) return MOVE_NONE;", "    const LevelMove *moves = g_learnsets[species];", "    for (u8 i = 0; i < MAX_LEARNSET_MOVES; i++) {", "        if (moves[i].move == MOVE_NONE) break;", "        if (moves[i].level == level) return moves[i].move;", "    }", "    return MOVE_NONE;", "}", ""]
    evolution_lines += ["};", ""]
    (ROOT / "src" / "data" / "learnsets.c").write_text("\n".join(learnset_lines), encoding="utf-8")
    (ROOT / "src" / "data" / "evolutions.c").write_text("\n".join(evolution_lines), encoding="utf-8")
    print(f"generated {len(names)} learnsets (max {max_moves}) and evolution records (max {max_evos})")


if __name__ == "__main__":
    main()
