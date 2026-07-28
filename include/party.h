#pragma once

#include "types.h"
#include "pokemon.h"
#include "moves.h"

#define PARTY_SIZE 6
#define PARTY_NICKNAME_LENGTH 8

typedef struct {
    PokemonId species;
    u8 level;
    u16 dv;
    u16 max_hp;
    u16 current_hp;
    u16 attack;
    u16 defense;
    u16 speed;
    u16 special;
    MoveId moves[4];
    u8 pp[4];
    u8 status;
    char nickname[PARTY_NICKNAME_LENGTH];
    u32 experience;
} PartyPokemon;

typedef struct {
    u8 count;
    PartyPokemon mons[PARTY_SIZE];
} PartyState;

typedef struct {
    u8 map_id;
    u8 x;
    u8 y;
    u8 facing;
} HealingPoint;

extern PartyState g_party;
extern HealingPoint g_last_healing_point;

void party_clear(void);
void party_set_starter(PokemonId species, const char *nickname);
void party_set_active_nickname(const char *nickname);
PartyPokemon *party_get_active(void);
PartyPokemon *party_get_lead(void);
PartyPokemon *party_get_slot(u8 slot);
const char    *party_mon_display_name(const PartyPokemon *mon);
void party_export(PartyState *out);
void party_import(const PartyState *in);
void party_update_active(const PartyPokemon *mon);
void party_update_slot(u8 slot, const PartyPokemon *mon);
void party_evolve_slot(u8 slot, PokemonId species);
void party_evolve_active(PokemonId species);
void party_swap_slots(u8 a, u8 b);
bool8 party_add(const PartyPokemon *mon);
bool8 party_has_usable_mon(u8 exclude_slot);
void party_heal_all(void);
bool8 party_all_fainted(void);
void party_set_healing_point(u8 map_id, u8 x, u8 y, u8 facing);
