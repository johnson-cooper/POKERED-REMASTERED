#pragma once

#include "types.h"

// OBJ tile ranges: player 0-23, Pokéball 24-27, Blue 28-51, Oak 52-75.
#define GFX_BLUE_TILE_BASE 28
#define GFX_OAK_TILE_BASE  52

extern const u32 g_blue_tiles[];
extern const u32 g_blue_tile_count;
extern const u32 g_oak_tiles[];
extern const u32 g_oak_tile_count;
