#include "gba.h"
#include "indoor_palette.h"

// Indoor palette set from pokemon-rgb/color/data/map_palettes.asm.
// All three Pallet Town interiors share these 8 palettes; visual
// differences come from the per-tileset tile_palette_map assignments.
//
// GB shade mapping to GBA 4bpp indices:
//   shade 3 (darkest)  -> index 0 + index 1
//   shade 2 (dark)     -> index 5
//   shade 1 (light)    -> index 10
//   shade 0 (lightest) -> index 15
// Index 0 uses the darkest shade (not transparent black) because indoor
// tilesets blank tile 0 for screen clearing, and wall metatiles reference
// tile 0 — its pixels must render as the darkest wall shade.

#define E 0x0000

const u16 g_indoor_palette_colors[8 * 16] = {
    // 0 INDOOR_GRAY
    RGB15( 7, 7, 7), RGB15( 7, 7, 7),E,E,E, RGB15(13,13,13), E,E,E,E, RGB15(19,19,19), E,E,E,E, RGB15(30,28,26),
    // 1 INDOOR_RED
    RGB15( 7, 7, 7), RGB15( 7, 7, 7),E,E,E, RGB15(30,10, 6), E,E,E,E, RGB15(31,19,24), E,E,E,E, RGB15(30,28,26),
    // 2 INDOOR_GREEN
    RGB15( 7, 7, 7), RGB15( 7, 7, 7),E,E,E, RGB15( 9,13, 0), E,E,E,E, RGB15(15,20, 1), E,E,E,E, RGB15(30,28,26),
    // 3 INDOOR_BLUE (WATER)
    RGB15( 7, 7, 7), RGB15( 7, 7, 7),E,E,E, RGB15( 9, 9,31), E,E,E,E, RGB15(15,16,31), E,E,E,E, RGB15(30,28,26),
    // 4 INDOOR_YELLOW
    RGB15( 7, 7, 7), RGB15( 7, 7, 7),E,E,E, RGB15(31,16, 1), E,E,E,E, RGB15(31,31, 7), E,E,E,E, RGB15(30,28,26),
    // 5 INDOOR_BROWN
    RGB15( 7, 7, 7), RGB15( 7, 7, 7),E,E,E, RGB15(16,13, 3), E,E,E,E, RGB15(21,17, 7), E,E,E,E, RGB15(30,28,26),
    // 6 INDOOR_LIGHT_BLUE (ROOF slot)
    RGB15( 7, 7, 7), RGB15( 7, 7, 7),E,E,E, RGB15(14,16,31), E,E,E,E, RGB15(17,19,31), E,E,E,E, RGB15(30,28,26),
    // 7 CRYS_TEXTBOX
    RGB15( 0, 0, 0), RGB15( 0, 0, 0),E,E,E, RGB15(31,31,31), E,E,E,E, RGB15(31,31,31), E,E,E,E, RGB15(31,31,31),
};

#undef E

const PaletteProfile g_indoor_palette_profile = {
    .colors = g_indoor_palette_colors,
    .count = 8,
};

// Per-tile palette assignments from pokemon-rgb/color/tilesets/.
// GRAY=0  RED=1  GREEN=2  WATER=3  YELLOW=4  BROWN=5  ROOF=6  TEXT=7

const u8 g_reds_house_tile_palette_map[96] = {
    3,0,0,0,1,0,3,3,   // 0x00-0x07
    2,2,5,5,5,5,0,0,   // 0x08-0x0F
    0,0,0,0,1,0,3,3,   // 0x10-0x17
    5,5,5,5,5,5,0,0,   // 0x18-0x1F
    0,0,5,5,3,3,5,5,   // 0x20-0x27
    5,5,5,5,5,0,0,0,   // 0x28-0x2F
    5,5,5,5,3,3,5,5,   // 0x30-0x37
    5,5,5,5,5,0,0,0,   // 0x38-0x3F
    0,0,0,0,2,2,5,5,   // 0x40-0x47
    0,0,0,0,0,0,0,0,   // 0x48-0x4F
    0,0,0,0,0,0,0,0,   // 0x50-0x57
    0,0,0,0,0,0,0,0,   // 0x58-0x5F
};

const u8 g_house_tile_palette_map[96] = {
    3,0,0,0,1,0,3,3,   // 0x00-0x07
    2,2,2,2,2,2,5,5,   // 0x08-0x0F
    0,0,0,0,1,0,3,3,   // 0x10-0x17
    5,5,5,5,0,0,5,5,   // 0x18-0x1F
    0,0,3,3,3,5,5,5,   // 0x20-0x27
    5,5,2,2,5,3,3,5,   // 0x28-0x2F
    5,5,5,5,3,5,5,5,   // 0x30-0x37
    5,5,5,5,5,3,3,3,   // 0x38-0x3F
    0,0,0,0,0,0,5,5,   // 0x40-0x47
    3,3,3,3,5,5,5,5,   // 0x48-0x4F
    5,5,5,5,3,3,5,5,   // 0x50-0x57
    3,0,3,3,5,5,5,5,   // 0x58-0x5F
};

const u8 g_gym_tile_palette_map[96] = {
    3,0,5,1,0,0,1,5,   // 0x00-0x07
    5,0,0,0,0,5,5,0,   // 0x08-0x0F
    3,0,5,5,3,0,1,5,   // 0x10-0x17
    5,0,0,0,0,5,5,0,   // 0x18-0x1F
    0,0,0,0,6,6,6,6,   // 0x20-0x27
    0,5,5,2,2,2,2,2,   // 0x28-0x2F
    0,0,0,0,3,6,0,0,   // 0x30-0x37
    5,5,5,5,1,1,3,1,   // 0x38-0x3F
    2,2,3,3,0,0,0,0,   // 0x40-0x47
    0,0,0,0,1,1,5,5,   // 0x48-0x4F
    2,2,3,3,0,0,0,0,   // 0x50-0x57
    5,5,5,0,0,0,0,0,   // 0x58-0x5F
};
