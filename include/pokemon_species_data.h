#pragma once
#include "pokemon.h"
#include "moves.h"

extern const MoveId g_pokemon_initial_moves[NUM_POKEMON + 1][4];
const MoveId *pokemon_initial_moves(PokemonId species);
