#pragma once
#include "types.h"
#include "pokemon.h"

// Map script system — called once per frame from world_update() via the
// map header's script function pointer.
//
// Each map defines its own static script state and script function.
// Scripts interact with the dialog system and game flags.

// NPC A-press interaction: called from player_update when player presses A
// facing an NPC at the adjacent subtile.
void script_trigger_npc(u16 script_id, u8 npc_index);
bool8 script_viridian_old_man_blocks(s32 x, s32 y);

// Advances generic NPC dialogs on every map, including maps without a
// dedicated map-script callback.
void script_update(void);

// Called from world_update before player input, to let active scripts
// block input (e.g. scripted walks, dialog).
bool8 script_blocks_input(void);
bool8 script_oaks_lab_blocks_exit(u8 dir);
PokemonId script_get_starter_species(void);
void script_reset_runtime(void);

// Pallet Town and Oak's Lab script functions (referenced by MapHeader.script)
void script_pallet_town(void);
void script_oaks_lab(void);
void script_viridian_city(void);
void script_viridian_mart(void);
