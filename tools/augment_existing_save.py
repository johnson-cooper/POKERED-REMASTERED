#!/usr/bin/env python3
"""Migrate an existing playable save into the current v5 test layout."""
from __future__ import annotations

import ctypes
import runpy
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "22pokered_remaster.sav"
OUTPUT = ROOT / "pokered_remaster_from_current.sav"

def main() -> None:
    module = runpy.run_path(str(ROOT / "tools" / "create_test_save.py"))
    SaveData = module["SaveData"]
    PartyPokemon = module["PartyPokemon"]
    species_data = module["species_data"]
    make_mon = module["make_mon"]
    PC_BOX_SIZE = module["PC_BOX_SIZE"]
    NUM_POKEMON = module["NUM_POKEMON"]
    raw = SOURCE.read_bytes()
    save = SaveData()

    # The header, map state, flags, options, party, healing point, money,
    # bag, and Pokédex fields precede the old PC region and retain the
    # player's existing progress. The current PC begins at offset 480.
    common_size = SaveData.pc.offset
    ctypes.memmove(ctypes.addressof(save), raw[:common_size], common_size)

    data = species_data()
    for box in range(len(save.pc.boxes)):
        save.pc.boxes[box].count = 0
        for i in range(PC_BOX_SIZE):
            save.pc.boxes[box].mons[i] = PartyPokemon()
    for species in range(1, NUM_POKEMON + 1):
        box, index = divmod(species - 1, PC_BOX_SIZE)
        save.pc.boxes[box].mons[index] = make_mon(species, 50, data)
        save.pc.boxes[box].count = index + 1
    save.pc.current_box = 0

    # Keep the existing party if it is valid; otherwise provide a test party.
    if save.party.count == 0 or save.party.count > 6:
        save.party.count = 1
        save.party.mons[0] = make_mon(1, 50, data)

    save.money = 999999
    save.bag.count = 0
    for slot in save.bag.slots:
        slot.id = 0
        slot.quantity = 0
    items = [(8, 10), (9, 10), (10, 10), (11, 10), (12, 10),
             (1, 99), (2, 99), (3, 99), (4, 99), (5, 99),
             (13, 1), (14, 1), (15, 1), (16, 1), (17, 1),
             (63, 1), (64, 1), (65, 1), (66, 1), (67, 1)]
    for i, (item, quantity) in enumerate(items):
        save.bag.slots[i].id = item
        save.bag.slots[i].quantity = quantity
    save.bag.count = len(items)

    save.pc.item_count = 50
    for i in range(50):
        save.pc.items[i].id = 13 + i
        save.pc.items[i].quantity = 1

    for i in range(20):
        save.pokedex_seen[i] = 0xFF
        save.pokedex_owned[i] = 0xFF

    raw_out = bytearray(bytes(save))
    raw_out[0:4] = b"RRM1"
    raw_out[4] = 5
    checksum_offset = SaveData.checksum.offset
    raw_out[checksum_offset] = sum(raw_out[4:checksum_offset]) & 0xFF
    OUTPUT.write_bytes(raw_out + bytes(32768 - len(raw_out)))
    print(f"created {OUTPUT} from {SOURCE.name}")
    print(f"checksum offset={checksum_offset}, checksum={raw_out[checksum_offset]}, size={len(raw_out)}")

if __name__ == "__main__":
    main()
