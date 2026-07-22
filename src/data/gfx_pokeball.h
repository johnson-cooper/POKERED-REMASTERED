#pragma once

#include "types.h"

// The player currently occupies OBJ tiles 0-23. The Pokéball is a
// four-tile (16x16) object sprite loaded immediately after it.
#define GFX_POKEBALL_TILE_BASE 24

extern const u32 g_pokeball_tiles[];
extern const u32 g_pokeball_tile_count;
