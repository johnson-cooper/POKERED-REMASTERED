#include "gba.h"
#include "overworld_palette_map.h"

// Palette assignments based on pokemon-rgb/color/tilesets/overworld.asm,
// These assignments match pokemon-rgb/color/tilesets/overworld.asm exactly.
// In particular, house corner/background, fence, and sign tiles retain the
// source tile palettes instead of inheriting a whole-house override.
// GRAY=0 RED=1 GREEN=2 WATER=3 YELLOW=4 BROWN=5 ROOF=6 TEXT=7
const u8 g_overworld_tile_palette_map[96] = {
    0,5,5,1,5,6,6,6,   // 0x00-0x07
    6,6,4,4,4,0,5,0,   // 0x08-0x0F
    0,5,6,5,3,6,6,6,   // 0x10-0x17
    6,6,0,5,5,2,5,0,   // 0x18-0x1F
    0,0,0,0,5,6,6,5,   // 0x20-0x27
    6,6,0,0,2,2,2,0,   // 0x28-0x2F
    5,0,5,5,5,5,5,5,   // 0x30-0x37
    6,0,0,0,5,2,2,0,   // 0x38-0x3F
    2,2,0,0,0,0,0,0,   // 0x40-0x47
    5,5,0,5,6,6,0,0,   // 0x48-0x4F
    2,2,2,6,5,5,0,0,   // 0x50-0x57
    5,5,6,5,6,6,0,0,   // 0x58-0x5F
};

// Source shade order is 0,5,A,F (dark-to-light). The source repository's
// palettes are light-to-dark, so reverse them into GBA indices 1,5,10,15.
#define E 0x0000
const u16 g_overworld_palette_colors[13 * 16] = {
    // OUTDOOR_GRAY (MapPaletteSets palette slot 0)
    RGB15(0,0,0), RGB15( 7, 7, 7),E,E,E, RGB15(13,13,13), E,E,E,E, RGB15(21,21,21), E,E,E,E, RGB15(27,31,27),
    RGB15(0,0,0), RGB15( 7, 7, 7),E,E,E, RGB15(30,10, 6), E,E,E,E, RGB15(31,19,24), E,E,E,E, RGB15(27,31,27),
    RGB15(0,0,0), RGB15( 7, 7, 7),E,E,E, RGB15( 5,14, 0), E,E,E,E, RGB15(12,25, 1), E,E,E,E, RGB15(22,31,10),
    RGB15(0,0,0), RGB15( 7, 7, 7),E,E,E, RGB15(13,12,31), E,E,E,E, RGB15(18,19,31), E,E,E,E, RGB15(23,23,31),
    RGB15(0,0,0), RGB15( 7, 7, 7),E,E,E, RGB15(31,16, 1), E,E,E,E, RGB15(31,31, 7), E,E,E,E, RGB15(27,31,27),
    RGB15(0,0,0), RGB15( 7, 7, 7),E,E,E, RGB15(20,15, 3), E,E,E,E, RGB15(24,18, 7), E,E,E,E, RGB15(27,31,27),
    RGB15(0,0,0), RGB15( 7, 7, 7),E,E,E, RGB15(11,23, 5), E,E,E,E, RGB15(20,31,14), E,E,E,E, RGB15(27,31,27),
    RGB15(0,0,0), RGB15( 0, 0, 0),E,E,E, RGB15(31,31,31), E,E,E,E, RGB15(31,31,31), E,E,E,E, RGB15(31,31,31),
    // HOUSE_WALL: warm cream with a dark enough base to preserve outlines.
    RGB15(0,0,0), RGB15(12,10, 7),E,E,E, RGB15(20,18,13), E,E,E,E, RGB15(27,25,20), E,E,E,E, RGB15(31,31,27),
    // HOUSE_ROOF: muted terracotta that contrasts cleanly with green grass.
    RGB15(0,0,0), RGB15(10, 3, 2),E,E,E, RGB15(18, 6, 3), E,E,E,E, RGB15(26,10, 4), E,E,E,E, RGB15(31,18, 8),
    // LAB_ROOF: slightly darker brown for visual separation from houses.
    RGB15(0,0,0), RGB15( 4, 2, 1),E,E,E, RGB15(10, 5, 2), E,E,E,E, RGB15(19,10, 4), E,E,E,E, RGB15(27,19, 9),
    // HOUSE_BLUE: light blue and white, shared by house walls and roofs.
    RGB15(0,0,0), RGB15( 8,16,24),E,E,E, RGB15(14,23,31), E,E,E,E, RGB15(23,29,31), E,E,E,E, RGB15(31,31,31),
};
#undef E

const PaletteProfile g_overworld_palette_profile = {
    .colors = g_overworld_palette_colors,
    .count = 13,
};

// Per-town roof palettes from pokemon-rgb/color/data/roofpalettes.asm.
// The reference provides the two lightest shades per town; the dark-mid
// shade is derived to match the hue so the roof stripe pattern blends in.
//                                        lightest          light             dark              darkest
// pokemon-rgb/color/data/roofpalettes.asm: PalletRoof. The source provides
// the two light shades; the darker shades remain the overworld roof colors.
const RoofPalette g_roof_pallet     = { RGB15(27,31,27), RGB15(24,24,24), RGB15(15,15,15), RGB15( 7, 7, 7) };
const RoofPalette g_roof_viridian   = { RGB15( 0,29, 7), RGB15( 0,24, 7), RGB15(11,23, 5), RGB15( 7, 7, 7) };
const RoofPalette g_roof_pewter     = { RGB15(24,25,26), RGB15(20,17,19), RGB15(12,10,11), RGB15( 7, 7, 7) };
const RoofPalette g_roof_cerulean   = { RGB15(14,24,31), RGB15(14,20,26), RGB15( 7,12,18), RGB15( 7, 7, 7) };
const RoofPalette g_roof_lavender   = { RGB15(23,12,31), RGB15(19, 9,24), RGB15(11, 5,16), RGB15( 7, 7, 7) };
const RoofPalette g_roof_vermilion  = { RGB15(29, 8, 0), RGB15(22, 8, 0), RGB15(14, 4, 0), RGB15( 7, 7, 7) };
const RoofPalette g_roof_celadon    = { RGB15(15,26,19), RGB15( 3,20,11), RGB15( 1,12, 6), RGB15( 7, 7, 7) };
const RoofPalette g_roof_fuchsia    = { RGB15(31, 3,18), RGB15(25, 3,12), RGB15(15, 1, 7), RGB15( 7, 7, 7) };
const RoofPalette g_roof_cinnabar   = { RGB15(29, 0, 0), RGB15(22, 0, 0), RGB15(13, 0, 0), RGB15( 7, 7, 7) };
const RoofPalette g_roof_indigo     = { RGB15(16, 0,31), RGB15(10, 0,25), RGB15( 6, 0,14), RGB15( 7, 7, 7) };
const RoofPalette g_roof_saffron    = { RGB15(31,27, 0), RGB15(28,22, 0), RGB15(16,12, 0), RGB15( 7, 7, 7) };
