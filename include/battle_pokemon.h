#pragma once
#include "types.h"
#include "pokemon.h"
#include "moves.h"

#define STAT_STAGE_ATK 0
#define STAT_STAGE_DEF 1
#define STAT_STAGE_SPD 2
#define STAT_STAGE_SPC 3
#define STAT_STAGE_ACC 4
#define STAT_STAGE_EVA 5

// Persistent Gen 1 status bits. Only one major status can be active at once;
// confusion is tracked separately by the battle flow when implemented.
#define STATUS_NONE      0x00
#define STATUS_POISON    0x01
#define STATUS_BURN      0x02
#define STATUS_SLEEP     0x04
#define STATUS_PARALYZE  0x08
#define STATUS_FREEZE    0x10
#define STATUS_CONFUSION 0x20

typedef struct {
    PokemonId species;
    u8  level;
    u16 dv;
    u16 max_hp, current_hp;
    u16 attack, defense, speed, special;
    MoveId moves[4];
    u8  pp[4];
    u8  status;
    s8  stages[6];
    const char *nickname;
} BattlePokemon;

void battle_pokemon_init(BattlePokemon *mon, PokemonId species, u8 level,
                         u16 dv, const MoveId moves[], u8 move_count);

u16 battle_stat_with_stage(u16 base_stat, s8 stage);

u8  dv_attack(u16 dv);
u8  dv_defense(u16 dv);
u8  dv_speed(u16 dv);
u8  dv_special(u16 dv);
u8  dv_hp(u16 dv);

extern const u8 g_stat_stage_num[13];
extern const u8 g_stat_stage_den[13];
