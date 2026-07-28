#pragma once

#include "gba.h"
#include "pokemon.h"

void pokemon_palette_load(vu16 *dst, PokemonId species);
void pokemon_battle_palette_load(vu16 *dst, PokemonId species);
