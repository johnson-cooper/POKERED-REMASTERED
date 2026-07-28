#include "trainer_parties.h"

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

static const TrainerParty s_bug_catcher_forest[] = {
    { "BUG CATCHER", 2,
      {{ MON_WEEDLE, 6 }, { MON_CATERPIE, 6 }, { MON_NONE, 0 }} },
    { "BUG CATCHER", 3,
      {{ MON_WEEDLE, 7 }, { MON_KAKUNA, 7 }, { MON_WEEDLE, 7 }, { MON_NONE, 0 }} },
    { "BUG CATCHER", 1,
      {{ MON_WEEDLE, 9 }, { MON_NONE, 0 }} },
};

const TrainerParty *trainer_party_get(TrainerId trainer, u8 variant) {
    if (trainer <= TRAINER_NONE || trainer >= TRAINER_COUNT)
        return NULL;
    if (trainer == TRAINER_BUG_CATCHER &&
        variant < ARRAY_COUNT(s_bug_catcher_forest))
        return &s_bug_catcher_forest[variant];
    return &g_trainer_parties[trainer];
}
