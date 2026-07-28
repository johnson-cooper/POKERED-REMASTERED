#pragma once

#include "types.h"
#include "pokemon.h"

typedef PokemonId PokedexSpecies;

#define POKEDEX_BULBASAUR  MON_BULBASAUR
#define POKEDEX_CHARMANDER MON_CHARMANDER
#define POKEDEX_SQUIRTLE   MON_SQUIRTLE
#define POKEDEX_PIDGEY     MON_PIDGEY
#define POKEDEX_RATTATA    MON_RATTATA
#define POKEDEX_NIDORINO   MON_NIDORINO
#define POKEDEX_SPEAROW    MON_SPEAROW
#define POKEDEX_NIDORAN_F  MON_NIDORAN_F
#define POKEDEX_NIDORAN_M  MON_NIDORAN_M
#define POKEDEX_WEEDLE     MON_WEEDLE
#define POKEDEX_ENTRY_COUNT (NUM_POKEMON + 1)

typedef struct {
    const char *name;
    const char *category;
    const char *height;
    const char *weight;
    const char *number;
    const char *description[3];
    const char *description_page2[3];
} PokedexEntry;

extern const PokedexEntry g_pokedex_entries[NUM_POKEMON + 1];

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
