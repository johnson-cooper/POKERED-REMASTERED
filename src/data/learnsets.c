#include "learnsets.h"

const PokemonLearnset g_learnsets[NUM_POKEMON + 1] = {
    [MON_CHARMANDER] = {
        {  9, MOVE_EMBER },
        { 15, MOVE_LEER },
        { 22, MOVE_RAGE },
        { 30, MOVE_SLASH },
        { 38, MOVE_FLAMETHROWER },
        { 46, MOVE_FIRE_SPIN },
        {  0, MOVE_NONE },
    },
};
