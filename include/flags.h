#pragma once
#include "types.h"

typedef enum {
    FLAG_GOT_STARTER = 0,
    FLAG_OAK_APPEARED_IN_PALLET,
    FLAG_FOLLOWED_OAK_INTO_LAB,
    FLAG_OAK_ASKED_TO_CHOOSE_MON,
    FLAG_BATTLED_RIVAL_IN_OAKS_LAB,
    FLAG_RIVAL_LEFT_OAKS_LAB,
    FLAG_GOT_POKEDEX,
    // Append persistent lab-object flags so existing save bits retain their
    // meanings when loading a save created by an earlier build.
    FLAG_OAKSLAB_CHARMANDER_TAKEN,
    FLAG_OAKSLAB_SQUIRTLE_TAKEN,
    FLAG_OAKSLAB_BULBASAUR_TAKEN,
    FLAG_STARTER_CHARMANDER,
    FLAG_STARTER_SQUIRTLE,
    FLAG_STARTER_BULBASAUR,
    FLAG_GOT_OAKS_PARCEL,
    FLAG_OAK_GOT_PARCEL,
    FLAG_GOT_TOWN_MAP,
    FLAG_GOT_ROUTE1_POTION,
    FLAG_ROUTE22_RIVAL_WANTS_BATTLE,
    FLAG_BEAT_ROUTE22_RIVAL,
    FLAG_ROUTE2_MOON_STONE,
    FLAG_ROUTE2_HP_UP,
    FLAG_VIRIDIAN_FOREST_ANTIDOTE,
    FLAG_VIRIDIAN_FOREST_POTION,
    FLAG_VIRIDIAN_FOREST_POKE_BALL,
    FLAG_VIRIDIAN_FOREST_TRAINER_0,
    FLAG_VIRIDIAN_FOREST_TRAINER_1,
    FLAG_VIRIDIAN_FOREST_TRAINER_2,
    FLAG_COUNT,
} GameFlag;

void  flags_clear_all(void);
void  flags_set(GameFlag f);
void  flags_clear(GameFlag f);
bool8 flags_get(GameFlag f);
void  flags_export(u32 out[4]);
void  flags_import(const u32 in[4]);
