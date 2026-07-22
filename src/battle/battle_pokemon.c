#include "battle_pokemon.h"

u8 dv_attack(u16 dv)  { return (u8)((dv >> 12) & 0xF); }
u8 dv_defense(u16 dv) { return (u8)((dv >> 8)  & 0xF); }
u8 dv_speed(u16 dv)   { return (u8)((dv >> 4)  & 0xF); }
u8 dv_special(u16 dv) { return (u8)(dv & 0xF); }

u8 dv_hp(u16 dv) {
    return (u8)(((dv_attack(dv) & 1) << 3) |
                ((dv_defense(dv) & 1) << 2) |
                ((dv_speed(dv) & 1) << 1) |
                (dv_special(dv) & 1));
}

static u16 calc_hp(u8 base, u8 iv, u8 level) {
    return (u16)(((u32)(base + iv) * 2 * level) / 100 + level + 10);
}

static u16 calc_stat(u8 base, u8 iv, u8 level) {
    return (u16)(((u32)(base + iv) * 2 * level) / 100 + 5);
}

void battle_pokemon_init(BattlePokemon *mon, PokemonId species, u8 level,
                         u16 dv, const MoveId moves[], u8 move_count) {
    const PokemonBaseStats *base = &g_pokemon_base_stats[species];

    mon->species = species;
    mon->level = level;
    mon->dv = dv;

    mon->max_hp  = calc_hp(base->hp, dv_hp(dv), level);
    mon->current_hp = mon->max_hp;
    mon->attack  = calc_stat(base->attack,  dv_attack(dv),  level);
    mon->defense = calc_stat(base->defense, dv_defense(dv), level);
    mon->speed   = calc_stat(base->speed,   dv_speed(dv),   level);
    mon->special = calc_stat(base->special, dv_special(dv), level);

    for (u8 i = 0; i < 4; i++) {
        if (i < move_count && moves[i] != MOVE_NONE) {
            mon->moves[i] = moves[i];
            mon->pp[i] = g_move_data[moves[i]].pp;
        } else {
            mon->moves[i] = MOVE_NONE;
            mon->pp[i] = 0;
        }
    }

    mon->status = STATUS_NONE;
    for (u8 i = 0; i < 6; i++)
        mon->stages[i] = 0;
    mon->nickname = NULL;
}

u16 battle_stat_with_stage(u16 base_stat, s8 stage) {
    u8 idx = (u8)(stage + 6);
    if (idx > 12) idx = (stage < 0) ? 0 : 12;
    return (u16)((u32)base_stat * g_stat_stage_num[idx] / g_stat_stage_den[idx]);
}
