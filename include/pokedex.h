#pragma once

#include "types.h"
#include "pokemon.h"

typedef enum {
    POKEDEX_BULBASAUR = 0,
    POKEDEX_CHARMANDER,
    POKEDEX_SQUIRTLE,
    POKEDEX_PIDGEY,
    POKEDEX_RATTATA,
    POKEDEX_NIDORINO,
    POKEDEX_SPEAROW,
    POKEDEX_NIDORAN_F,
    POKEDEX_NIDORAN_M,
    POKEDEX_ENTRY_COUNT,
} PokedexSpecies;

void  pokedex_open(PokedexSpecies species);
bool8 pokedex_update(void);
void  pokedex_close(void);
bool8 pokedex_species_to_entry(PokemonId species, PokedexSpecies *out);

void  pokedex_set_seen(PokemonId species);
void  pokedex_set_owned(PokemonId species);
bool8 pokedex_is_seen(PokemonId species);
bool8 pokedex_is_owned(PokemonId species);
u16   pokedex_seen_count(void);
u16   pokedex_owned_count(void);
void  pokedex_tracking_clear(void);
void  pokedex_tracking_export(u8 seen[20], u8 owned[20]);
void  pokedex_tracking_import(const u8 seen[20], const u8 owned[20]);

void  pokedex_list_open(void);
bool8 pokedex_list_update(void);
void  pokedex_list_close(void);
