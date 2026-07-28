#!/usr/bin/env python3
"""Generate species-wide initial moves from pokered base-stats data."""
from __future__ import annotations
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = ROOT / "refs" / "pokered"

def source_names() -> list[str]:
    text = (REFERENCE / "data" / "pokemon" / "base_stats.asm").read_text(encoding="utf-8", errors="replace")
    names = re.findall(r'INCLUDE "data/pokemon/base_stats/([^".]+)\.asm"', text)
    if len(names) != 150: raise RuntimeError(f"expected 150 species before Mew, got {len(names)}")
    return names + ["mew"]

def enum_name(name: str) -> str:
    return {"nidoranf": "MON_NIDORAN_F", "nidoranm": "MON_NIDORAN_M", "mrmime": "MON_MR_MIME"}.get(name, "MON_" + name.upper())

def move_name(name: str) -> str:
    return {"NO_MOVE": "MOVE_NONE", "PSYCHIC_M": "MOVE_PSYCHIC"}.get(name.upper(), "MOVE_" + name.upper())

def main() -> None:
    lines = ['#include "pokemon_species_data.h"', "", "const MoveId g_pokemon_initial_moves[NUM_POKEMON + 1][4] = {", "    [MON_NONE] = { MOVE_NONE, MOVE_NONE, MOVE_NONE, MOVE_NONE },"]
    for name in source_names():
        text = (REFERENCE / "data" / "pokemon" / "base_stats" / f"{name}.asm").read_text(encoding="utf-8", errors="replace")
        match = re.search(r"^\s*db\s+(.+?)\s*; level 1 learnset", text, re.MULTILINE)
        if not match: raise RuntimeError(f"missing level 1 learnset for {name}")
        moves = [move_name(value.strip()) for value in match.group(1).split(",")]
        if len(moves) != 4: raise RuntimeError(f"expected four initial moves for {name}")
        lines.append(f"    [{enum_name(name)}] = {{ {', '.join(moves)} }},")
    lines += ["};", "", "const MoveId *pokemon_initial_moves(PokemonId species) {", "    if (species == MON_NONE || species > NUM_POKEMON) return g_pokemon_initial_moves[MON_NONE];", "    return g_pokemon_initial_moves[species];", "}", ""]
    (ROOT / "src" / "data" / "pokemon_species_data.c").write_text("\n".join(lines), encoding="utf-8")
    print(f"generated initial moves for {len(source_names())} species")

if __name__ == "__main__": main()
