#pragma once
#include "types.h"

typedef enum {
    FLAG_GOT_STARTER = 0,
    FLAG_OAK_APPEARED_IN_PALLET,
    FLAG_FOLLOWED_OAK_INTO_LAB,
    FLAG_OAK_ASKED_TO_CHOOSE_MON,
    FLAG_BATTLED_RIVAL_IN_OAKS_LAB,
    FLAG_COUNT,
} GameFlag;

void  flags_clear_all(void);
void  flags_set(GameFlag f);
void  flags_clear(GameFlag f);
bool8 flags_get(GameFlag f);
