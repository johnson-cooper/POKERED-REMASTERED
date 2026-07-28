#pragma once
#include "types.h"
#include "pokemon.h"

#define TRAINER_PARTY_MAX 6

typedef enum {
    TRAINER_NONE = 0,
    TRAINER_YOUNGSTER,
    TRAINER_BUG_CATCHER,
    TRAINER_COUNT,
} TrainerId;

typedef struct {
    PokemonId species;
    u8 level;
} TrainerPokemon;

typedef struct {
    const char *name;
    u8 count;
    TrainerPokemon party[TRAINER_PARTY_MAX];
} TrainerParty;

extern const TrainerParty g_trainer_parties[TRAINER_COUNT];
