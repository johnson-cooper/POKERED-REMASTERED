#include "flags.h"

static u32 s_flags[4];  // 128 bits

void flags_clear_all(void) {
    s_flags[0] = s_flags[1] = s_flags[2] = s_flags[3] = 0;
}

void flags_set(GameFlag f) {
    s_flags[f >> 5] |= (1u << (f & 31));
}

void flags_clear(GameFlag f) {
    s_flags[f >> 5] &= ~(1u << (f & 31));
}

bool8 flags_get(GameFlag f) {
    return (s_flags[f >> 5] >> (f & 31)) & 1;
}

void flags_export(u32 out[4]) {
    for (u8 i = 0; i < 4; i++) out[i] = s_flags[i];
}

void flags_import(const u32 in[4]) {
    for (u8 i = 0; i < 4; i++) s_flags[i] = in[i];
}
