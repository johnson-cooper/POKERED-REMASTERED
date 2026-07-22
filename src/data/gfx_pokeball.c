#include "gfx_pokeball.h"

// Pokémon Red's original 16x16 Pokéball sprite, converted to GBA 4bpp
// tiles. Palette values match the existing OBJ palette.
const u32 g_pokeball_tiles[32] = {
    // top-left 8x8
    0xFFFFFFFF, 0xFFFFFFFF, 0x00FFFFFF, 0x5500FFFF,
    0xA5550FFF, 0xA5550FFF, 0x555550FF, 0x555550FF,
    // top-right 8x8
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0055,
    0xFFF0555A, 0xFFF0555A, 0xFF055555, 0xFF055555,
    // bottom-left 8x8
    0x555AA0FF, 0xAAAAA0FF, 0xAAAA0FFF, 0xAAAA0FFF,
    0xAA00FFFF, 0x00FFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    // bottom-right 8x8
    0xFF0AA555, 0xFF0AAAAA, 0xFFF0AAAA, 0xFFF0AAAA,
    0xFFFF00AA, 0xFFFFFF00, 0xFFFFFFFF, 0xFFFFFFFF,
};

const u32 g_pokeball_tile_count = 4;
