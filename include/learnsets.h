#pragma once
#include "types.h"
#include "moves.h"
#include "pokemon.h"

#define MAX_LEARNSET_MOVES 10  // 9 real + sentinel

typedef struct {
    u8     level;
    MoveId move;
} LevelMove;

typedef LevelMove PokemonLearnset[MAX_LEARNSET_MOVES];

extern const PokemonLearnset g_learnsets[NUM_POKEMON + 1];
