#include "learnsets.h"

const PokemonLearnset g_learnsets[NUM_POKEMON + 1] = {
    [MON_BULBASAUR] = {
        {  7, MOVE_LEECH_SEED },
        { 13, MOVE_VINE_WHIP },
        { 20, MOVE_POISONPOWDER },
        { 27, MOVE_RAZOR_LEAF },
        { 34, MOVE_GROWTH },
        { 41, MOVE_SLEEP_POWDER },
        { 48, MOVE_SOLARBEAM },
        {  0, MOVE_NONE },
    },
    [MON_CHARMANDER] = {
        {  9, MOVE_EMBER },
        { 15, MOVE_LEER },
        { 22, MOVE_RAGE },
        { 30, MOVE_SLASH },
        { 38, MOVE_FLAMETHROWER },
        { 46, MOVE_FIRE_SPIN },
        {  0, MOVE_NONE },
    },
    [MON_SQUIRTLE] = {
        {  8, MOVE_BUBBLE },
        { 15, MOVE_WATER_GUN },
        { 22, MOVE_BITE },
        { 28, MOVE_WITHDRAW },
        { 35, MOVE_SKULL_BASH },
        { 42, MOVE_HYDRO_PUMP },
        {  0, MOVE_NONE },
    },
    [MON_NIDORAN_F] = {
        {  8, MOVE_SCRATCH },
        { 14, MOVE_POISON_STING },
        { 21, MOVE_TAIL_WHIP },
        { 29, MOVE_BITE },
        { 36, MOVE_FURY_SWIPES },
        { 43, MOVE_DOUBLE_KICK },
        {  0, MOVE_NONE },
    },
    [MON_NIDORAN_M] = {
        {  8, MOVE_HORN_ATTACK },
        { 14, MOVE_POISON_STING },
        { 21, MOVE_FOCUS_ENERGY },
        { 29, MOVE_FURY_ATTACK },
        { 36, MOVE_HORN_DRILL },
        { 43, MOVE_DOUBLE_KICK },
        {  0, MOVE_NONE },
    },
    [MON_NIDORINO] = {
        {  8, MOVE_HORN_ATTACK },
        { 14, MOVE_POISON_STING },
        { 23, MOVE_FOCUS_ENERGY },
        { 32, MOVE_FURY_ATTACK },
        { 41, MOVE_HORN_DRILL },
        { 50, MOVE_DOUBLE_KICK },
        {  0, MOVE_NONE },
    },
    [MON_PIDGEY] = {
        {  5, MOVE_SAND_ATTACK },
        { 12, MOVE_QUICK_ATTACK },
        { 19, MOVE_WHIRLWIND },
        { 28, MOVE_WING_ATTACK },
        { 36, MOVE_AGILITY },
        { 44, MOVE_MIRROR_MOVE },
        {  0, MOVE_NONE },
    },
    [MON_RATTATA] = {
        {  7, MOVE_QUICK_ATTACK },
        { 14, MOVE_HYPER_FANG },
        { 23, MOVE_FOCUS_ENERGY },
        { 34, MOVE_SUPER_FANG },
        {  0, MOVE_NONE },
    },
    [MON_SPEAROW] = {
        {  9, MOVE_LEER },
        { 15, MOVE_FURY_ATTACK },
        { 22, MOVE_MIRROR_MOVE },
        { 29, MOVE_DRILL_PECK },
        { 36, MOVE_AGILITY },
        {  0, MOVE_NONE },
    },
};

MoveId learnset_move_at_level(PokemonId species, u8 level) {
    if (species == MON_NONE || species > NUM_POKEMON) return MOVE_NONE;
    const LevelMove *moves = g_learnsets[species];
    for (u8 i = 0; i < MAX_LEARNSET_MOVES; i++) {
        if (moves[i].move == MOVE_NONE) break;
        if (moves[i].level == level) return moves[i].move;
    }
    return MOVE_NONE;
}
