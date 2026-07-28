#include "trainer_parties.h"

// These entries are intentionally not connected to maps yet. Future trainer
// scripts can select a TrainerId and the battle engine will load the party.
const TrainerParty g_trainer_parties[TRAINER_COUNT] = {
    [TRAINER_NONE] = { "TRAINER", 0, {{ MON_NONE, 0 }} },
    [TRAINER_YOUNGSTER] = {
        "YOUNGSTER", 1,
        {{ MON_RATTATA, 4 }, { MON_NONE, 0 }}
    },
    [TRAINER_BUG_CATCHER] = {
        "BUG CATCHER", 2,
        {{ MON_WEEDLE, 3 }, { MON_CATERPIE, 3 }, { MON_NONE, 0 }}
    },
};
