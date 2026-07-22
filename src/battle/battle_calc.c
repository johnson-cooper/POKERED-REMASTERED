#include "battle_calc.h"
#include "battle_rng.h"
#include "type_effectiveness.h"

bool8 battle_move_is_physical(PokemonType type) {
    return type <= TYPE_GHOST;
}

bool8 battle_check_critical(const BattlePokemon *atk, MoveId move) {
    (void)move;
    const PokemonBaseStats *base = &g_pokemon_base_stats[atk->species];
    u8 threshold = base->speed / 2;
    return (battle_random() & 0xFF) < threshold;
}

bool8 battle_check_hit(const BattlePokemon *atk, const BattlePokemon *def,
                       MoveId move) {
    const MoveData *md = &g_move_data[move];
    if (md->effect == EFFECT_SWIFT) return TRUE;

    u16 acc = md->accuracy;
    u16 stage_num = g_stat_stage_num[(u8)(atk->stages[STAT_STAGE_ACC] + 6)];
    u16 stage_den = g_stat_stage_den[(u8)(atk->stages[STAT_STAGE_ACC] + 6)];
    acc = acc * stage_num / stage_den;

    u16 eva_num = g_stat_stage_num[(u8)(def->stages[STAT_STAGE_EVA] + 6)];
    u16 eva_den = g_stat_stage_den[(u8)(def->stages[STAT_STAGE_EVA] + 6)];
    acc = acc * eva_den / eva_num;

    if (acc > 255) acc = 255;
    u8 threshold = (u8)((acc * 255) / 100);
    u8 roll = (u8)(battle_random() & 0xFF);
    return roll < threshold;
}

u16 battle_calc_damage(const BattlePokemon *atk, const BattlePokemon *def,
                       MoveId move, bool8 is_critical) {
    const MoveData *md = &g_move_data[move];
    const PokemonBaseStats *atk_base = &g_pokemon_base_stats[atk->species];
    const PokemonBaseStats *def_base = &g_pokemon_base_stats[def->species];

    if (md->power == 0) return 0;

    u16 atk_stat, def_stat;
    if (battle_move_is_physical(md->type)) {
        atk_stat = atk->attack;
        def_stat = def->defense;
    } else {
        atk_stat = atk->special;
        def_stat = def->special;
    }

    if (is_critical) {
        atk_stat = atk->attack;
        def_stat = def->defense;
        if (!battle_move_is_physical(md->type)) {
            atk_stat = atk->special;
            def_stat = def->special;
        }
    } else {
        if (battle_move_is_physical(md->type)) {
            atk_stat = battle_stat_with_stage(atk->attack, atk->stages[STAT_STAGE_ATK]);
            def_stat = battle_stat_with_stage(def->defense, def->stages[STAT_STAGE_DEF]);
        } else {
            atk_stat = battle_stat_with_stage(atk->special, atk->stages[STAT_STAGE_SPC]);
            def_stat = battle_stat_with_stage(def->special, def->stages[STAT_STAGE_SPC]);
        }
    }

    if (def_stat == 0) def_stat = 1;

    u8 level = atk->level;
    if (is_critical) level *= 2;

    u32 damage = ((u32)(2 * level / 5 + 2) * md->power * atk_stat / def_stat) / 50 + 2;

    if (md->type == atk_base->type1 || md->type == atk_base->type2)
        damage = damage * 3 / 2;

    u8 eff = type_effectiveness(md->type, def_base->type1, def_base->type2);
    damage = damage * eff / TYPE_MUL_NEUTRAL;

    if (eff == TYPE_MUL_NO_EFFECT) return 0;

    u16 rand_val = (u16)((battle_random() % 39) + 217);
    damage = damage * rand_val / 255;

    if (damage == 0) damage = 1;
    return (u16)damage;
}
