#pragma once

#include "types.h"
#include "pokemon.h"

#define BATTLE_SPRITE_TILES 64
#define BATTLE_SPRITE_WORDS (BATTLE_SPRITE_TILES * 8)

void battle_sprite_load_front(PokemonId species, u32 *dest);
void battle_sprite_load_back(PokemonId species, u32 *dest);
