#include "wild_encounters.h"
#include "map_ids.h"
#include "battle_rng.h"

const WildEncounterTable g_wild_encounter_tables[MAP_COUNT] = {
    [MAP_ROUTE_1] = {
        25,
        {
            { MON_PIDGEY, 3, 3, 50 }, { MON_RATTATA, 3, 3, 51 },
            { MON_RATTATA, 3, 3, 39 }, { MON_RATTATA, 2, 2, 25 },
            { MON_PIDGEY, 2, 2, 25 }, { MON_PIDGEY, 3, 3, 25 },
            { MON_PIDGEY, 3, 3, 13 }, { MON_RATTATA, 4, 4, 13 },
            { MON_PIDGEY, 4, 4, 11 }, { MON_PIDGEY, 5, 5, 4 },
        },
    },
    [MAP_ROUTE_2] = {
        25,
        {
            { MON_RATTATA, 3, 3, 50 }, { MON_PIDGEY, 3, 3, 51 },
            { MON_PIDGEY, 4, 4, 39 }, { MON_RATTATA, 4, 4, 25 },
            { MON_PIDGEY, 5, 5, 25 }, { MON_WEEDLE, 3, 3, 25 },
            { MON_RATTATA, 2, 2, 13 }, { MON_RATTATA, 5, 5, 13 },
            { MON_WEEDLE, 4, 4, 11 }, { MON_WEEDLE, 5, 5, 4 },
        },
    },
    [MAP_ROUTE_22] = {
        25,
        {
            { MON_RATTATA, 3, 3, 50 }, { MON_NIDORAN_M, 3, 3, 51 },
            { MON_RATTATA, 4, 4, 39 }, { MON_NIDORAN_M, 4, 4, 25 },
            { MON_RATTATA, 2, 2, 25 }, { MON_NIDORAN_M, 2, 2, 25 },
            { MON_SPEAROW, 3, 3, 13 }, { MON_SPEAROW, 5, 5, 13 },
            { MON_NIDORAN_F, 3, 3, 11 }, { MON_NIDORAN_F, 4, 4, 4 },
        },
    },
};

bool8 wild_encounter_select(u8 map_id, PokemonId *species, u8 *level) {
    if (map_id >= MAP_COUNT || !species || !level) return FALSE;
    const WildEncounterTable *table = &g_wild_encounter_tables[map_id];
    if (!table->encounter_rate ||
        (battle_random() & 0xFF) >= table->encounter_rate)
        return FALSE;

    u8 roll = (u8)(battle_random() & 0xFF);
    u8 slot = 0;
    while (slot < WILD_ENCOUNTER_SLOTS - 1 && roll >= table->slots[slot].weight) {
        roll = (u8)(roll - table->slots[slot].weight);
        slot++;
    }
    const WildEncounterSlot *entry = &table->slots[slot];
    *species = entry->species;
    u8 span = (u8)(entry->max_level - entry->min_level + 1);
    *level = (u8)(entry->min_level + (span > 1 ? battle_random() % span : 0));
    return TRUE;
}
