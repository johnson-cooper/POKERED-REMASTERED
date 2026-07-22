#pragma once
#include "types.h"
#include "pokemon.h"

#define TYPE_MUL_NO_EFFECT       0
#define TYPE_MUL_NOT_VERY        5
#define TYPE_MUL_NEUTRAL        10
#define TYPE_MUL_SUPER          20

typedef struct {
    PokemonType attacker;
    PokemonType defender;
    u8 multiplier;
} TypeMatchup;

extern const TypeMatchup g_type_matchups[];
extern const u16 g_type_matchup_count;

u8 type_effectiveness_single(PokemonType atk_type, PokemonType def_type);
u8 type_effectiveness(PokemonType atk_type, PokemonType def_type1, PokemonType def_type2);
