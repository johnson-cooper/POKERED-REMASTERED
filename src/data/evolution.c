#include "evolution.h"

const Evolution *pokemon_evolution_for_level(PokemonId species, u8 level) {
    if (species == MON_NONE || species > NUM_POKEMON) return 0;
    const Evolution *evolutions = g_pokemon_evolutions[species];
    for (u8 i = 0; i < MAX_EVOLUTIONS; i++) {
        if (evolutions[i].method == EVOLUTION_NONE) break;
        if (evolutions[i].method == EVOLUTION_LEVEL &&
            level >= evolutions[i].requirement) {
            return &evolutions[i];
        }
    }
    return 0;
}

const Evolution *pokemon_evolution_for_item(PokemonId species, ItemId item) {
    EvolutionItem evolution_item = EVO_ITEM_NONE;
    switch (item) {
    case ITEM_FIRE_STONE:     evolution_item = EVO_ITEM_FIRE_STONE; break;
    case ITEM_THUNDER_STONE:  evolution_item = EVO_ITEM_THUNDER_STONE; break;
    case ITEM_WATER_STONE:    evolution_item = EVO_ITEM_WATER_STONE; break;
    case ITEM_LEAF_STONE:     evolution_item = EVO_ITEM_LEAF_STONE; break;
    case ITEM_MOON_STONE:     evolution_item = EVO_ITEM_MOON_STONE; break;
    default: return 0;
    }
    if (species == MON_NONE || species > NUM_POKEMON) return 0;
    const Evolution *evolutions = g_pokemon_evolutions[species];
    for (u8 i = 0; i < MAX_EVOLUTIONS; i++) {
        if (evolutions[i].method == EVOLUTION_NONE) break;
        if (evolutions[i].method == EVOLUTION_ITEM &&
            evolutions[i].item == evolution_item)
            return &evolutions[i];
    }
    return 0;
}
