#pragma once
#include "types.h"
#include "pokemon.h"

#define WILD_ENCOUNTER_SLOTS 10

typedef struct {
    PokemonId species;
    u8 min_level;
    u8 max_level;
    u8 weight;
} WildEncounterSlot;

typedef struct {
    u8 encounter_rate;
    WildEncounterSlot slots[WILD_ENCOUNTER_SLOTS];
} WildEncounterTable;

extern const WildEncounterTable g_wild_encounter_tables[];

bool8 wild_encounter_select(u8 map_id, PokemonId *species, u8 *level);
