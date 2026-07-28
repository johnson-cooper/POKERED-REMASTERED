#pragma once

#include "types.h"

// The player currently occupies OBJ tiles 0-23. The Pokéball is a
// four-tile (16x16) object sprite loaded immediately after it.
#define GFX_POKEBALL_TILE_BASE 24

extern const u32 g_pokeball_tiles[];
extern const u32 g_pokeball_tile_count;

// Convert the source sprite's white background to GBA OBJ transparency.
static inline u32 remap_sprite_nibbles(u32 v) {
    u32 result = v;
    for (u32 shift = 0; shift < 32; shift += 4) {
        u32 n = (v >> shift) & 0xF;
        if (n == 0x0 || n == 0xF)
            result ^= (0xFu << shift);
    }
    return result;
}
