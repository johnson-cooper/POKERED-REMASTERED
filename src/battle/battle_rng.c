#include "battle_rng.h"

static u32 s_rng_state = 1;

void battle_rng_seed(u32 seed) {
    s_rng_state = seed ? seed : 1;
}

u16 battle_random(void) {
    s_rng_state = s_rng_state * 1103515245u + 12345u;
    return (u16)(s_rng_state >> 16);
}
