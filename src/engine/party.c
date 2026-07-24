#include "party.h"
#include "battle_pokemon.h"
#include "map_ids.h"

PartyState g_party;
HealingPoint g_last_healing_point;

static void healing_point_set_default(void) {
    g_last_healing_point.map_id = MAP_PLAYERS_HOUSE_1F;
    g_last_healing_point.x = 4;
    g_last_healing_point.y = 3;
    g_last_healing_point.facing = 3; // DIR_RIGHT
}

void party_clear(void) {
    for (u32 i = 0; i < sizeof(g_party); i++)
        ((u8 *)&g_party)[i] = 0;
    healing_point_set_default();
}

static void copy_nickname(char dst[PARTY_NICKNAME_LENGTH], const char *src) {
    for (u8 i = 0; i < PARTY_NICKNAME_LENGTH; i++) {
        dst[i] = src ? src[i] : '\0';
        if (!dst[i]) break;
    }
    dst[PARTY_NICKNAME_LENGTH - 1] = '\0';
}

void party_set_starter(PokemonId species, const char *nickname) {
    BattlePokemon mon;
    static const MoveId starter_moves[][2] = {
        [MON_BULBASAUR]  = { MOVE_TACKLE, MOVE_GROWL },
        [MON_CHARMANDER] = { MOVE_SCRATCH, MOVE_GROWL },
        [MON_SQUIRTLE]   = { MOVE_TACKLE, MOVE_TAIL_WHIP },
    };
    party_clear();
    battle_pokemon_init(&mon, species, 5, 0x9888,
                        starter_moves[species], 2);
    g_party.count = 1;
    g_party.mons[0].species = mon.species;
    g_party.mons[0].level = mon.level;
    g_party.mons[0].dv = mon.dv;
    g_party.mons[0].max_hp = mon.max_hp;
    g_party.mons[0].current_hp = mon.current_hp;
    g_party.mons[0].attack = mon.attack;
    g_party.mons[0].defense = mon.defense;
    g_party.mons[0].speed = mon.speed;
    g_party.mons[0].special = mon.special;
    for (u8 i = 0; i < 4; i++) {
        g_party.mons[0].moves[i] = mon.moves[i];
        g_party.mons[0].pp[i] = mon.pp[i];
    }
    g_party.mons[0].status = mon.status;
    copy_nickname(g_party.mons[0].nickname, nickname);
}

PartyPokemon *party_get_active(void) {
    return g_party.count ? &g_party.mons[0] : NULL;
}

void party_update_active(const PartyPokemon *mon) {
    if (g_party.count && mon)
        g_party.mons[0] = *mon;
}

void party_heal_all(void) {
    for (u8 i = 0; i < g_party.count && i < PARTY_SIZE; i++) {
        PartyPokemon *mon = &g_party.mons[i];
        mon->current_hp = mon->max_hp;
        mon->status = STATUS_NONE;
        for (u8 j = 0; j < 4; j++) {
            if (mon->moves[j] != MOVE_NONE)
                mon->pp[j] = g_move_data[mon->moves[j]].pp;
        }
    }
}

bool8 party_all_fainted(void) {
    if (!g_party.count) return FALSE;
    for (u8 i = 0; i < g_party.count && i < PARTY_SIZE; i++) {
        if (g_party.mons[i].current_hp > 0) return FALSE;
    }
    return TRUE;
}

void party_set_healing_point(u8 map_id, u8 x, u8 y, u8 facing) {
    g_last_healing_point.map_id = map_id;
    g_last_healing_point.x = x;
    g_last_healing_point.y = y;
    g_last_healing_point.facing = facing;
}

void party_export(PartyState *out) {
    if (out) *out = g_party;
}

void party_import(const PartyState *in) {
    if (in) {
        g_party = *in;
        if (g_party.count > PARTY_SIZE) g_party.count = PARTY_SIZE;
    }
}
