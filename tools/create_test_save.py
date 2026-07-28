#!/usr/bin/env python3
"""Create a fully populated v5 SRAM image for emulator testing."""
from __future__ import annotations

import ctypes
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REF = ROOT / "refs" / "pokered"
OUT = ROOT / "pokered_test_all_pokemon.sav"

NUM_POKEMON = 151
PARTY_SIZE = 6
PC_BOX_SIZE = 20
PC_NUM_BOXES = 12
PC_ITEM_SLOTS = 50
PARTY_NICKNAME_LENGTH = 8
POKEDEX_BYTES = 20

U8 = ctypes.c_ubyte
U16 = ctypes.c_ushort
U32 = ctypes.c_uint
# The ARM toolchain uses short enum storage for these bounded IDs.
ENUM = ctypes.c_ubyte

class PartyPokemon(ctypes.Structure):
    _fields_ = [("species", ENUM), ("level", U8), ("dv", U16),
                ("max_hp", U16), ("current_hp", U16), ("attack", U16),
                ("defense", U16), ("speed", U16), ("special", U16),
                ("moves", ENUM * 4), ("pp", U8 * 4), ("status", U8),
                ("nickname", ctypes.c_char * PARTY_NICKNAME_LENGTH),
                ("experience", U32)]

class PartyState(ctypes.Structure):
    _fields_ = [("count", U8), ("mons", PartyPokemon * PARTY_SIZE)]

class HealingPoint(ctypes.Structure):
    _fields_ = [("map_id", U8), ("x", U8), ("y", U8), ("facing", U8)]

class BagSlot(ctypes.Structure):
    _fields_ = [("id", U8), ("quantity", U8)]

class BagState(ctypes.Structure):
    _fields_ = [("count", U8), ("slots", BagSlot * 20)]

class PcBox(ctypes.Structure):
    _fields_ = [("count", U8), ("mons", PartyPokemon * PC_BOX_SIZE)]

class PcState(ctypes.Structure):
    _fields_ = [("current_box", U8), ("boxes", PcBox * PC_NUM_BOXES),
                ("item_count", U8), ("items", BagSlot * PC_ITEM_SLOTS)]

class SaveData(ctypes.Structure):
    _fields_ = [("magic", U8 * 4), ("version", U8), ("map_id", U8),
                ("last_map_id", U8), ("player_x", U8), ("player_y", U8),
                ("player_facing", U8), ("player_name", U8 * 8),
                ("rival_name", U8 * 8), ("flags", U32 * 4),
                ("option_fast_text", U8), ("option_battle_animation", U8),
                ("option_battle_style", U8), ("party", PartyState),
                ("last_healing_point", HealingPoint), ("money", U32),
                ("bag", BagState), ("pokedex_seen", U8 * POKEDEX_BYTES),
                ("pokedex_owned", U8 * POKEDEX_BYTES), ("pc", PcState),
                ("checksum", U8)]

def species_names():
    text = (REF / "data/pokemon/base_stats.asm").read_text(encoding="utf-8", errors="replace")
    return re.findall(r'INCLUDE "data/pokemon/base_stats/([^".]+)\.asm"', text) + ["mew"]

def move_ids():
    text = (ROOT / "include/moves.h").read_text(encoding="utf-8", errors="replace")
    block = text.split("    MOVE_NONE = 0,", 1)[1].split("} MoveId;", 1)[0]
    return {name: i for i, name in enumerate(["MOVE_NONE"] + re.findall(r"\bMOVE_[A-Z0-9_]+", block))}

def species_data():
    moves = move_ids()
    result = {}
    for species, name in enumerate(species_names(), 1):
        text = (REF / "data/pokemon/base_stats" / f"{name}.asm").read_text(encoding="utf-8", errors="replace")
        stats = tuple(map(int, re.search(r"^\s*db\s+(\d+),\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+)", text, re.M).groups()))
        raw_moves = re.search(r"^\s*db\s+(.+?)\s*; level 1 learnset", text, re.M).group(1).split(",")
        initial = []
        for raw in raw_moves:
            key = raw.strip().upper()
            if key == "NO_MOVE": key = "NONE"
            if key == "PSYCHIC_M": key = "PSYCHIC"
            key = "MOVE_" + key
            initial.append(moves[key])
        growth = re.search(r"GROWTH_(\w+)", text).group(1)
        result[species] = (stats, initial, growth)
    return result

def exp_for_level(growth: str, level: int) -> int:
    n, square = level, level * level
    cube = square * level
    return {"FAST": (4 * cube) // 5, "SLOW": (5 * cube) // 4,
            "MEDIUM_FAST": cube, "MEDIUM_SLOW": (6 * cube) // 5 - 15 * square + 100 * n - 140,
            "IRREGULAR": cube, "FLUCTUATING": cube}[growth]

def make_mon(species: int, level: int, data) -> PartyPokemon:
    base, moves, growth = data[species]
    mon = PartyPokemon()
    mon.species = species
    mon.level = level
    mon.dv = 0xFFFF
    hp, atk, defense, speed, special = base
    mon.max_hp = ((hp + 15) * 2 * level) // 100 + level + 10
    mon.current_hp = mon.max_hp
    mon.attack = ((atk + 15) * 2 * level) // 100 + 5
    mon.defense = ((defense + 15) * 2 * level) // 100 + 5
    mon.speed = ((speed + 15) * 2 * level) // 100 + 5
    mon.special = ((special + 15) * 2 * level) // 100 + 5
    for i, move in enumerate(moves):
        mon.moves[i] = move
        mon.pp[i] = 99 if move else 0
    mon.experience = exp_for_level(growth, level)
    return mon

def put_item(bag: BagState, slot: int, item: int, quantity: int) -> int:
    bag.slots[slot].id = item
    bag.slots[slot].quantity = quantity
    return slot + 1

def main() -> None:
    data = species_data()
    save = SaveData()
    save.magic[:] = b"RRM1"
    save.version = 5
    save.map_id = 20       # Route 1
    save.last_map_id = 20
    save.player_x = 11      # Route 1 grass area
    save.player_y = 7
    save.player_facing = 3
    save.player_name[:] = b"TEST\0\0\0\0"
    save.rival_name[:] = b"RIVAL\0\0\0"
    save.flags[0] = (1 << 0) | (1 << 6) | (1 << 12) | (1 << 15)
    save.option_fast_text = 1
    save.option_battle_animation = 1
    save.option_battle_style = 0
    save.money = 999999
    save.last_healing_point.map_id = 40
    save.last_healing_point.x = 4
    save.last_healing_point.y = 3
    save.last_healing_point.facing = 3

    save.party.count = 6
    for i in range(6): save.party.mons[i] = make_mon(i + 1, 50, data)
    for species in range(1, NUM_POKEMON + 1):
        box, index = divmod(species - 1, PC_BOX_SIZE)
        save.pc.boxes[box].mons[index] = make_mon(species, 50, data)
        save.pc.boxes[box].count = index + 1
    save.pc.current_box = 0

    slot = 0
    for item, quantity in [(8, 10), (9, 10), (10, 10), (11, 10), (12, 10),
                           (1, 99), (2, 99), (3, 99), (4, 99), (5, 99),
                           (13, 1), (14, 1), (15, 1), (16, 1), (17, 1),
                           (63, 1), (64, 1), (65, 1), (66, 1), (67, 1)]:
        slot = put_item(save.bag, slot, item, quantity)
    save.bag.count = slot
    save.pc.item_count = 50
    for i in range(50):
        save.pc.items[i].id = 13 + i  # TM01 through TM50 in the PC
        save.pc.items[i].quantity = 1
    for species in range(1, NUM_POKEMON + 1):
        index = species - 1
        save.pokedex_seen[index // 8] |= 1 << (index % 8)
        save.pokedex_owned[index // 8] |= 1 << (index % 8)

    raw = bytearray(bytes(save))
    raw[0:4] = b"RRM1"
    raw[4] = 5
    checksum_offset = SaveData.checksum.offset
    raw[checksum_offset] = sum(raw[4:checksum_offset]) & 0xFF
    OUT.write_bytes(raw + bytes(32768 - len(raw)))
    print(f"created {OUT} ({len(raw)} bytes payload, 32768-byte SRAM image)")
    print(f"SaveData size: {ctypes.sizeof(SaveData)}, PC species: {NUM_POKEMON}, bag slots: {save.bag.count}")

if __name__ == "__main__": main()
