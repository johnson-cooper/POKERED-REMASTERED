#!/usr/bin/env python3
"""Generate Gen 1 TM/HM moves and species compatibility from pokered."""
from __future__ import annotations
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REF = ROOT / "refs" / "pokered"

def names():
    text = (REF / "data/pokemon/base_stats.asm").read_text(encoding="utf-8", errors="replace")
    return re.findall(r'INCLUDE "data/pokemon/base_stats/([^".]+)\.asm"', text) + ["mew"]

def mon(name):
    return {"nidoranf":"MON_NIDORAN_F", "nidoranm":"MON_NIDORAN_M", "mrmime":"MON_MR_MIME"}.get(name, "MON_" + name.upper())

def move(name):
    return {"PSYCHIC_M":"MOVE_PSYCHIC"}.get(name, "MOVE_" + name)

def machine_moves():
    text = (REF / "constants/item_constants.asm").read_text(encoding="utf-8", errors="replace")
    return [move(x) for x in re.findall(r'^\s*add_tm\s+([A-Z0-9_]+)', text, re.M)] + [move(x) for x in re.findall(r'^\s*add_hm\s+([A-Z0-9_]+)', text, re.M)]

def main():
    machines = machine_moves()
    if len(machines) != 55: raise RuntimeError(f"expected 55 machines, got {len(machines)}")
    lines = ['#include "tmhm.h"', "", "const MoveId g_tmhm_moves[NUM_TM_HM] = {", "    " + ", ".join(machines) + ",", "};", "", "const u8 g_pokemon_tmhm_compat[NUM_POKEMON + 1][7] = {"]
    for name in names():
        text = (REF / "data/pokemon/base_stats" / f"{name}.asm").read_text(encoding="utf-8", errors="replace")
        section = re.search(r'(?ms)^\s*tmhm\s+(.*?)^\s*; end', text)
        if not section: raise RuntimeError(f"missing tmhm data for {name}")
        allowed = set(re.findall(r'[A-Z][A-Z0-9_]+', section.group(1)))
        bits = []
        for start in range(0, 55, 8):
            value = sum((1 << bit) for bit in range(8) if start + bit < 55 and re.sub(r'^MOVE_', '', machines[start + bit]).replace('PSYCHIC', 'PSYCHIC_M') in allowed)
            bits.append(f"0x{value:02X}")
        lines.append(f"    [{mon(name)}] = {{ {', '.join(bits)} }},")
    lines += ["};", "", "MoveId tmhm_move(u8 number) {", "    if (number == 0 || number > NUM_TM_HM) return MOVE_NONE;", "    return g_tmhm_moves[number - 1];", "}", "", "bool8 pokemon_can_learn_tmhm(PokemonId species, u8 number) {", "    if (species == MON_NONE || species > NUM_POKEMON || number == 0 || number > NUM_TM_HM) return FALSE;", "    u8 index = (u8)(number - 1);", "    return (g_pokemon_tmhm_compat[species][index >> 3] & (1 << (index & 7))) != 0;", "}", ""]
    (ROOT / "include/tmhm.h").write_text((ROOT / "include/tmhm.h").read_text(encoding="utf-8").replace("extern const MoveId g_tmhm_moves[NUM_TM_HM];", "extern const MoveId g_tmhm_moves[NUM_TM_HM];\nextern const u8 g_pokemon_tmhm_compat[NUM_POKEMON + 1][7];"), encoding="utf-8")
    (ROOT / "src/data/tmhm.c").write_text("\n".join(lines), encoding="utf-8")
    print(f"generated {len(names())} species TM/HM compatibility records")

if __name__ == "__main__": main()
