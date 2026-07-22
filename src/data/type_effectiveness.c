#include "type_effectiveness.h"

const TypeMatchup g_type_matchups[] = {
    { TYPE_WATER,    TYPE_FIRE,     TYPE_MUL_SUPER },
    { TYPE_FIRE,     TYPE_GRASS,    TYPE_MUL_SUPER },
    { TYPE_FIRE,     TYPE_ICE,      TYPE_MUL_SUPER },
    { TYPE_GRASS,    TYPE_WATER,    TYPE_MUL_SUPER },
    { TYPE_ELECTRIC, TYPE_WATER,    TYPE_MUL_SUPER },
    { TYPE_WATER,    TYPE_ROCK,     TYPE_MUL_SUPER },
    { TYPE_GROUND,   TYPE_FLYING,   TYPE_MUL_NO_EFFECT },
    { TYPE_WATER,    TYPE_WATER,    TYPE_MUL_NOT_VERY },
    { TYPE_FIRE,     TYPE_FIRE,     TYPE_MUL_NOT_VERY },
    { TYPE_ELECTRIC, TYPE_ELECTRIC, TYPE_MUL_NOT_VERY },
    { TYPE_ICE,      TYPE_ICE,      TYPE_MUL_NOT_VERY },
    { TYPE_GRASS,    TYPE_GRASS,    TYPE_MUL_NOT_VERY },
    { TYPE_PSYCHIC,  TYPE_PSYCHIC,  TYPE_MUL_NOT_VERY },
    { TYPE_FIRE,     TYPE_WATER,    TYPE_MUL_NOT_VERY },
    { TYPE_GRASS,    TYPE_FIRE,     TYPE_MUL_NOT_VERY },
    { TYPE_WATER,    TYPE_GRASS,    TYPE_MUL_NOT_VERY },
    { TYPE_ELECTRIC, TYPE_GRASS,    TYPE_MUL_NOT_VERY },
    { TYPE_NORMAL,   TYPE_ROCK,     TYPE_MUL_NOT_VERY },
    { TYPE_NORMAL,   TYPE_GHOST,    TYPE_MUL_NO_EFFECT },
    { TYPE_GHOST,    TYPE_GHOST,    TYPE_MUL_SUPER },
    { TYPE_FIRE,     TYPE_BUG,      TYPE_MUL_SUPER },
    { TYPE_FIRE,     TYPE_ROCK,     TYPE_MUL_NOT_VERY },
    { TYPE_WATER,    TYPE_GROUND,   TYPE_MUL_SUPER },
    { TYPE_ELECTRIC, TYPE_GROUND,   TYPE_MUL_NO_EFFECT },
    { TYPE_ELECTRIC, TYPE_FLYING,   TYPE_MUL_SUPER },
    { TYPE_GRASS,    TYPE_GROUND,   TYPE_MUL_SUPER },
    { TYPE_GRASS,    TYPE_BUG,      TYPE_MUL_NOT_VERY },
    { TYPE_GRASS,    TYPE_POISON,   TYPE_MUL_NOT_VERY },
    { TYPE_GRASS,    TYPE_ROCK,     TYPE_MUL_SUPER },
    { TYPE_GRASS,    TYPE_FLYING,   TYPE_MUL_NOT_VERY },
    { TYPE_ICE,      TYPE_WATER,    TYPE_MUL_NOT_VERY },
    { TYPE_ICE,      TYPE_GRASS,    TYPE_MUL_SUPER },
    { TYPE_ICE,      TYPE_GROUND,   TYPE_MUL_SUPER },
    { TYPE_ICE,      TYPE_FLYING,   TYPE_MUL_SUPER },
    { TYPE_FIGHTING, TYPE_NORMAL,   TYPE_MUL_SUPER },
    { TYPE_FIGHTING, TYPE_POISON,   TYPE_MUL_NOT_VERY },
    { TYPE_FIGHTING, TYPE_FLYING,   TYPE_MUL_NOT_VERY },
    { TYPE_FIGHTING, TYPE_PSYCHIC,  TYPE_MUL_NOT_VERY },
    { TYPE_FIGHTING, TYPE_BUG,      TYPE_MUL_NOT_VERY },
    { TYPE_FIGHTING, TYPE_ROCK,     TYPE_MUL_SUPER },
    { TYPE_FIGHTING, TYPE_ICE,      TYPE_MUL_SUPER },
    { TYPE_FIGHTING, TYPE_GHOST,    TYPE_MUL_NO_EFFECT },
    { TYPE_POISON,   TYPE_GRASS,    TYPE_MUL_SUPER },
    { TYPE_POISON,   TYPE_POISON,   TYPE_MUL_NOT_VERY },
    { TYPE_POISON,   TYPE_GROUND,   TYPE_MUL_NOT_VERY },
    { TYPE_POISON,   TYPE_BUG,      TYPE_MUL_SUPER },
    { TYPE_POISON,   TYPE_ROCK,     TYPE_MUL_NOT_VERY },
    { TYPE_POISON,   TYPE_GHOST,    TYPE_MUL_NOT_VERY },
    { TYPE_GROUND,   TYPE_FIRE,     TYPE_MUL_SUPER },
    { TYPE_GROUND,   TYPE_ELECTRIC, TYPE_MUL_SUPER },
    { TYPE_GROUND,   TYPE_GRASS,    TYPE_MUL_NOT_VERY },
    { TYPE_GROUND,   TYPE_BUG,      TYPE_MUL_NOT_VERY },
    { TYPE_GROUND,   TYPE_ROCK,     TYPE_MUL_SUPER },
    { TYPE_GROUND,   TYPE_POISON,   TYPE_MUL_SUPER },
    { TYPE_FLYING,   TYPE_ELECTRIC, TYPE_MUL_NOT_VERY },
    { TYPE_FLYING,   TYPE_FIGHTING, TYPE_MUL_SUPER },
    { TYPE_FLYING,   TYPE_BUG,      TYPE_MUL_SUPER },
    { TYPE_FLYING,   TYPE_GRASS,    TYPE_MUL_SUPER },
    { TYPE_FLYING,   TYPE_ROCK,     TYPE_MUL_NOT_VERY },
    { TYPE_PSYCHIC,  TYPE_FIGHTING, TYPE_MUL_SUPER },
    { TYPE_PSYCHIC,  TYPE_POISON,   TYPE_MUL_SUPER },
    { TYPE_BUG,      TYPE_FIRE,     TYPE_MUL_NOT_VERY },
    { TYPE_BUG,      TYPE_GRASS,    TYPE_MUL_SUPER },
    { TYPE_BUG,      TYPE_FIGHTING, TYPE_MUL_NOT_VERY },
    { TYPE_BUG,      TYPE_FLYING,   TYPE_MUL_NOT_VERY },
    { TYPE_BUG,      TYPE_PSYCHIC,  TYPE_MUL_SUPER },
    { TYPE_BUG,      TYPE_GHOST,    TYPE_MUL_NOT_VERY },
    { TYPE_BUG,      TYPE_POISON,   TYPE_MUL_SUPER },
    { TYPE_ROCK,     TYPE_FIRE,     TYPE_MUL_SUPER },
    { TYPE_ROCK,     TYPE_FIGHTING, TYPE_MUL_NOT_VERY },
    { TYPE_ROCK,     TYPE_GROUND,   TYPE_MUL_NOT_VERY },
    { TYPE_ROCK,     TYPE_FLYING,   TYPE_MUL_SUPER },
    { TYPE_ROCK,     TYPE_BUG,      TYPE_MUL_SUPER },
    { TYPE_ROCK,     TYPE_ICE,      TYPE_MUL_SUPER },
    { TYPE_GHOST,    TYPE_NORMAL,   TYPE_MUL_NO_EFFECT },
    { TYPE_GHOST,    TYPE_PSYCHIC,  TYPE_MUL_NO_EFFECT },
    { TYPE_FIRE,     TYPE_DRAGON,   TYPE_MUL_NOT_VERY },
    { TYPE_WATER,    TYPE_DRAGON,   TYPE_MUL_NOT_VERY },
    { TYPE_ELECTRIC, TYPE_DRAGON,   TYPE_MUL_NOT_VERY },
    { TYPE_GRASS,    TYPE_DRAGON,   TYPE_MUL_NOT_VERY },
    { TYPE_ICE,      TYPE_DRAGON,   TYPE_MUL_SUPER },
    { TYPE_DRAGON,   TYPE_DRAGON,   TYPE_MUL_SUPER },
};

const u16 g_type_matchup_count = ARRAY_COUNT(g_type_matchups);

u8 type_effectiveness_single(PokemonType atk_type, PokemonType def_type) {
    for (u16 i = 0; i < g_type_matchup_count; i++) {
        if (g_type_matchups[i].attacker == atk_type &&
            g_type_matchups[i].defender == def_type)
            return g_type_matchups[i].multiplier;
    }
    return TYPE_MUL_NEUTRAL;
}

u8 type_effectiveness(PokemonType atk_type, PokemonType def_type1, PokemonType def_type2) {
    u8 mul1 = type_effectiveness_single(atk_type, def_type1);
    if (def_type1 == def_type2)
        return mul1;
    u8 mul2 = type_effectiveness_single(atk_type, def_type2);
    return (u8)((u16)mul1 * mul2 / TYPE_MUL_NEUTRAL);
}
