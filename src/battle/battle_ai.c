#include "battle_ai.h"
#include "battle_rng.h"

MoveId battle_ai_choose_move(const BattlePokemon *mon) {
    u8 valid[4];
    u8 count = 0;

    for (u8 i = 0; i < 4; i++) {
        if (mon->moves[i] != MOVE_NONE && mon->pp[i] > 0)
            valid[count++] = i;
    }

    if (count == 0) return MOVE_STRUGGLE;
    u8 pick = (u8)(battle_random() % count);
    return mon->moves[valid[pick]];
}
