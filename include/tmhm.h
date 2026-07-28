#pragma once
#include "types.h"
#include "pokemon.h"
#include "moves.h"

#define NUM_TMS 50
#define NUM_HMS 5
#define NUM_TM_HM (NUM_TMS + NUM_HMS)

extern const MoveId g_tmhm_moves[NUM_TM_HM];
extern const u8 g_pokemon_tmhm_compat[NUM_POKEMON + 1][7];

MoveId tmhm_move(u8 number);
bool8 pokemon_can_learn_tmhm(PokemonId species, u8 number);
