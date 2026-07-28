#pragma once
#include "types.h"
#include "pokemon.h"
#include "item.h"

typedef enum {
    EVOLUTION_NONE = 0,
    EVOLUTION_LEVEL,
    EVOLUTION_ITEM,
    EVOLUTION_TRADE,
} EvolutionMethod;

typedef enum {
    EVO_ITEM_NONE = 0,
    EVO_ITEM_FIRE_STONE,
    EVO_ITEM_THUNDER_STONE,
    EVO_ITEM_WATER_STONE,
    EVO_ITEM_LEAF_STONE,
    EVO_ITEM_MOON_STONE,
} EvolutionItem;

typedef struct {
    EvolutionMethod method;
    u8 requirement;
    EvolutionItem item;
    PokemonId target;
} Evolution;

#define MAX_EVOLUTIONS 4  // 3 records + sentinel
typedef Evolution PokemonEvolution[MAX_EVOLUTIONS];

extern const PokemonEvolution g_pokemon_evolutions[NUM_POKEMON + 1];

const Evolution *pokemon_evolution_for_level(PokemonId species, u8 level);
const Evolution *pokemon_evolution_for_item(PokemonId species, ItemId item);
