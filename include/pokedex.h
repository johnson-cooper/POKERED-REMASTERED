#pragma once

#include "types.h"

typedef enum {
    POKEDEX_BULBASAUR = 0,
    POKEDEX_CHARMANDER,
    POKEDEX_SQUIRTLE,
} PokedexSpecies;

void  pokedex_open(PokedexSpecies species);
bool8 pokedex_update(void);
void  pokedex_close(void);
