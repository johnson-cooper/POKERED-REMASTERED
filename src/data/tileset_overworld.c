// Overworld tileset: Pokemon Red tile graphics converted from pokered's
// gfx/tilesets/overworld.png. The block definitions below are copied from
// pokered/gfx/blocksets/overworld.bst for Pallet Town's original block IDs.
#include "world.h"
#include "gba.h"
#include "metatile_ids.h"
#include "gfx_overworld.h"

// grit maps the four original GB shades to nibbles 0, 5, A, F.
// These GBA palettes recolor that same original art without changing pixels.
#define E 0x0000
static const u16 s_palettes[9 * 16] = {
    // 0: Grass
    RGB15( 1, 6, 0), RGB15( 1, 6, 0),E,E,E, RGB15( 7,18, 4), E,E,E,E,
    RGB15(13,25, 8), E,E,E,E, RGB15(21,30,13),

    // 1: Path / road
    RGB15( 8, 6, 3), RGB15( 8, 6, 3),E,E,E, RGB15(18,16,10), E,E,E,E,
    RGB15(24,22,16), E,E,E,E, RGB15(30,28,22),

    // 2: Building walls / stone
    RGB15( 6, 5, 3), RGB15( 6, 5, 3),E,E,E, RGB15(15,13, 9), E,E,E,E,
    RGB15(23,21,16), E,E,E,E, RGB15(26,24,19),

    // 3: Roofs. The house is intentionally monochrome now; keep this
    // separate bank for future roof styling, but match the wall color.
    RGB15( 6, 5, 3), RGB15( 6, 5, 3),E,E,E, RGB15(15,13, 9), E,E,E,E,
    RGB15(23,21,16), E,E,E,E, RGB15(26,24,19),

    // 4: Trees / deep foliage
    RGB15( 0, 2, 0), RGB15( 0, 2, 0),E,E,E, RGB15( 4,10, 2), E,E,E,E,
    RGB15( 8,18, 5), E,E,E,E, RGB15(14,24, 9),

    // 5: Signs / fence posts / wood
    RGB15( 5, 3, 1), RGB15( 5, 3, 1),E,E,E, RGB15(13, 9, 4), E,E,E,E,
    RGB15(22,16, 8), E,E,E,E, RGB15(30,24,14),

    // 6: Water
    RGB15( 0, 2, 7), RGB15( 0, 2, 7),E,E,E, RGB15( 2, 7,16), E,E,E,E,
    RGB15( 5,13,23), E,E,E,E, RGB15( 9,18,28),

    // 7: Flowers / light decorative plants
    RGB15( 8, 7, 1), RGB15( 8, 7, 1),E,E,E, RGB15(18,16, 5), E,E,E,E,
    RGB15(27,24,10), E,E,E,E, RGB15(31,31,22),

    // 8: Shoreline transition. The original tile mixes dark separators,
    // blue water shading, and light foam; it cannot use the all-blue water
    // palette without losing that structure.
    RGB15( 0, 5, 1), RGB15( 0, 5, 1),E,E,E, RGB15( 2,10,18), E,E,E,E,
    RGB15( 5,16,26), E,E,E,E, RGB15(14,24,31),
};
#undef E

#define P_GRASS   0
#define P_PATH    1
#define P_WALL    2
#define P_ROOF    3
#define P_TREE    4
#define P_WOOD    5
#define P_WATER   6
#define P_FLOWER  7
#define P_SHORE   8

// Palette assignment follows the semantic tile groups in overworld.png.
// The block geometry remains the original Red data; only the four GB shades
// are recolored for GBA. Keeping this per tile prevents a sign, roof, or
// water edge from inheriting the surrounding ground palette.
#define PAL_TILE(t) \
    (((t) == 0x14) ? P_WATER : \
     ((t) == 0x20 || (t) == 0x36 || (t) == 0x54) ? P_WOOD : \
     ((t) == 0x0E || (t) == 0x2E || (t) == 0x2F || (t) == 0x37 || \
      (t) == 0x46 || (t) == 0x47 || (t) == 0x55 || (t) == 0x56 || (t) == 0x57) ? P_WOOD : \
     ((t) == 0x2A || (t) == 0x2B || (t) == 0x3A || (t) == 0x3B) ? P_TREE : \
     ((t) == 0x0A || (t) == 0x0F || (t) == 0x1A || (t) == 0x20 || (t) == 0x22 || \
      (t) == 0x25 || (t) == 0x26 || (t) == 0x2C || (t) == 0x33 || (t) == 0x4E) ? P_PATH : \
     P_GRASS)

#define O(t) ((t) + OVERWORLD_OVERLAY_EDGE_MASK_BASE)
#define OE(t) ((t) + OVERWORLD_OVERLAY_EDGE_MASK_BASE)
#define OS(t) ((t) + OVERWORLD_OVERLAY_SOLID_BASE)

// Only the four outer roof-corner tiles contain outside grass inside their
// 8x8 cells. Every other house tile is opaque; using the edge mask on those
// tiles makes legitimate light wall pixels transparent and lets grass show
// through the building.
#define OH(t) (((t) == 0x05 || (t) == 0x06 || (t) == 0x08 || (t) == 0x09) ? O(t) : OS(t))

// 4x4 Red map block, row-major. top[] is currently unused/transparent.
#define BLK( \
            t00,t01,t02,t03, t10,t11,t12,t13, \
            t20,t21,t22,t23, t30,t31,t32,t33, \
            p00,p01,p02,p03, p10,p11,p12,p13, \
            p20,p21,p22,p23, p30,p31,p32,p33) \
    { .bottom={ \
        (t00),(t01),(t02),(t03), (t10),(t11),(t12),(t13), \
        (t20),(t21),(t22),(t23), (t30),(t31),(t32),(t33) }, \
      .top={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}, \
      .palettes={ \
        (p00),(p01),(p02),(p03), (p10),(p11),(p12),(p13), \
        (p20),(p21),(p22),(p23), (p30),(p31),(p32),(p33) }, \
      .top_palettes={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0} }

#define BLK2( \
             b00,b01,b02,b03, b10,b11,b12,b13, \
             b20,b21,b22,b23, b30,b31,b32,b33, \
             t00,t01,t02,t03, t10,t11,t12,t13, \
             t20,t21,t22,t23, t30,t31,t32,t33, \
             p00,p01,p02,p03, p10,p11,p12,p13, \
             p20,p21,p22,p23, p30,p31,p32,p33, \
             q00,q01,q02,q03, q10,q11,q12,q13, \
             q20,q21,q22,q23, q30,q31,q32,q33) \
    { .bottom={ \
        (b00),(b01),(b02),(b03), (b10),(b11),(b12),(b13), \
        (b20),(b21),(b22),(b23), (b30),(b31),(b32),(b33) }, \
      .top={ \
        (t00),(t01),(t02),(t03), (t10),(t11),(t12),(t13), \
        (t20),(t21),(t22),(t23), (t30),(t31),(t32),(t33) }, \
      .palettes={ \
        (p00),(p01),(p02),(p03), (p10),(p11),(p12),(p13), \
        (p20),(p21),(p22),(p23), (p30),(p31),(p32),(p33) }, \
      .top_palettes={ \
        (q00),(q01),(q02),(q03), (q10),(q11),(q12),(q13), \
        (q20),(q21),(q22),(q23), (q30),(q31),(q32),(q33) } }

// A full object block. BG2 uses the object palette (stone colors) so the base
// layer is never green. BG1 carries the solid overlay of the same palette,
// which remaps index-0 pixels (transparent on BG2) to index-1 (opaque stone).
#define BLK_OBJ(pbase,pobj, \
               t00,t01,t02,t03, t10,t11,t12,t13, \
               t20,t21,t22,t23, t30,t31,t32,t33) \
    { .bottom={ \
        OS(t00),OS(t01),OS(t02),OS(t03), OS(t10),OS(t11),OS(t12),OS(t13), \
        OS(t20),OS(t21),OS(t22),OS(t23), OS(t30),OS(t31),OS(t32),OS(t33) }, \
      .top={ \
        OS(t00),OS(t01),OS(t02),OS(t03), OS(t10),OS(t11),OS(t12),OS(t13), \
        OS(t20),OS(t21),OS(t22),OS(t23), OS(t30),OS(t31),OS(t32),OS(t33) }, \
      .palettes={ PAL_ALL(pobj) }, \
      .top_palettes={ PAL_ALL(pobj) } }

#define PAL_ALL(p) \
    (p),(p),(p),(p), (p),(p),(p),(p), \
    (p),(p),(p),(p), (p),(p),(p),(p)

#define BLK_ALL(pal, \
                t00,t01,t02,t03, t10,t11,t12,t13, \
                t20,t21,t22,t23, t30,t31,t32,t33) \
    { .bottom={ \
        (t00),(t01),(t02),(t03), (t10),(t11),(t12),(t13), \
        (t20),(t21),(t22),(t23), (t30),(t31),(t32),(t33) }, \
      .top={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}, \
      .palettes={ PAL_ALL(pal) }, \
      .top_palettes={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0} }

#define BLK_AUTO( \
                t00,t01,t02,t03, t10,t11,t12,t13, \
                t20,t21,t22,t23, t30,t31,t32,t33) \
    { .bottom={ (t00),(t01),(t02),(t03), (t10),(t11),(t12),(t13), \
                (t20),(t21),(t22),(t23), (t30),(t31),(t32),(t33) }, \
      .top={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}, \
      .palettes={ PAL_TILE(t00),PAL_TILE(t01),PAL_TILE(t02),PAL_TILE(t03), \
                  PAL_TILE(t10),PAL_TILE(t11),PAL_TILE(t12),PAL_TILE(t13), \
                  PAL_TILE(t20),PAL_TILE(t21),PAL_TILE(t22),PAL_TILE(t23), \
                  PAL_TILE(t30),PAL_TILE(t31),PAL_TILE(t32),PAL_TILE(t33) }, \
      .top_palettes={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0} }

static const Metatile s_metatiles[120] = {
    [MT_GRASS_STEP ] = BLK2(0x0A,0x0A,0x28,0x29, 0x4B,0x4B,0x4B,0x1F, 0x0A,0x0A,0x0A,0x1F, 0x1A,0x1A,0x1A,0x4F,
                             0,0,0,0, O(0x4B),O(0x4B),O(0x4B),0, 0,0,0,0, 0,0,0,0,
                             P_PATH,P_PATH,P_WALL,P_WALL, P_WALL,P_WALL,P_WALL,P_WALL,
                             P_PATH,P_PATH,P_PATH,P_WALL, P_PATH,P_PATH,P_PATH,P_PATH,
                             0,0,0,0, P_WALL,P_WALL,P_WALL,0, 0,0,0,0, 0,0,0,0),
    [MT_GRASS      ] = BLK_AUTO(
                               0x23,0x23,0x23,0x23, 0x39,0x23,0x23,0x23, 0x23,0x23,0x23,0x23, 0x23,0x23,0x39,0x23),
    [MT_TREE_CORNER] = BLK_AUTO(
                               0x39,0x39,0x39,0x39, 0x39,0x39,0x39,0x39, 0x39,0x39,0x46,0x47, 0x39,0x39,0x56,0x57),
    [MT_PATH_WALK  ] = BLK_AUTO(
                               0x2C,0x2C,0x2C,0x2C, 0x2C,0x2C,0x2C,0x2C, 0x2C,0x2C,0x2C,0x2C, 0x2C,0x2C,0x2C,0x2C),
    [MT_BORDER_TL  ] = BLK_AUTO(
                               0x52,0x52,0x52,0x52, 0x52,0x52,0x52,0x52, 0x52,0x52,0x52,0x52, 0x52,0x52,0x52,0x52),

    [MT_BLDG_LEFT  ] = BLK_OBJ(P_GRASS,P_WALL,
                               0x05,0x06,0x53,0x53, 0x15,0x38,0x12,0x12,
                               0x15,0x38,0x12,0x12, 0x15,0x16,0x17,0x17),
    [MT_BLDG_MID   ] = BLK_OBJ(P_GRASS,P_WALL,
                               0x53,0x53,0x53,0x53, 0x12,0x12,0x12,0x12,
                               0x12,0x12,0x12,0x12, 0x17,0x17,0x17,0x17),
    [MT_BLDG_RIGHT ] = BLK_OBJ(P_GRASS,P_WALL,
                               0x53,0x53,0x08,0x09, 0x12,0x12,0x38,0x19,
                               0x12,0x12,0x38,0x19, 0x17,0x17,0x18,0x19),
    [MT_DIRT       ] = BLK_AUTO(
                               0x25,0x26,0x0A,0x0A, 0x0F,0x22,0x22,0x22, 0x0F,0x0A,0x0A,0x0A, 0x4E,0x1A,0x1A,0x1A),
    [MT_WALKWAY_L  ] = BLK(0x33,0x33,0x33,0x33, 0x32,0x14,0x14,0x14, 0x32,0x14,0x14,0x14, 0x32,0x14,0x14,0x14,
                           P_WOOD,P_WOOD,P_WOOD,P_WOOD, P_WOOD,P_WATER,P_WATER,P_WATER,
                           P_WOOD,P_WATER,P_WATER,P_WATER, P_WOOD,P_WATER,P_WATER,P_WATER),
    [MT_WALKWAY_R  ] = BLK(0x33,0x33,0x33,0x33, 0x14,0x14,0x14,0x54, 0x14,0x14,0x14,0x54, 0x14,0x14,0x14,0x54,
                           P_WOOD,P_WOOD,P_WOOD,P_WOOD, P_WATER,P_WATER,P_WATER,P_WOOD,
                           P_WATER,P_WATER,P_WATER,P_WOOD, P_WATER,P_WATER,P_WATER,P_WOOD),
    [MT_PATH_CORNER] = BLK_AUTO(
                               0x39,0x39,0x39,0x39, 0x39,0x39,0x39,0x39, 0x39,0x39,0x39,0x39, 0x39,0x39,0x39,0x39),

    [MT_BLDG_TOP_L ] = BLK2(0x23,0x23,0x23,0x23, 0x39,0x23,0x23,0x23, 0x05,0x06,0x07,0x07, 0x15,0x16,0x17,0x17,
                             0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
                             P_GRASS,P_GRASS,P_GRASS,P_GRASS, P_GRASS,P_GRASS,P_GRASS,P_GRASS,
                             P_WALL,P_WALL,P_WALL,P_WALL, P_WALL,P_WALL,P_WALL,P_WALL,
                             0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0),
    [MT_BLDG_TOP_R ] = BLK2(0x23,0x23,0x23,0x23, 0x39,0x23,0x23,0x23, 0x07,0x07,0x08,0x09, 0x17,0x17,0x18,0x19,
                             0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
                             P_GRASS,P_GRASS,P_GRASS,P_GRASS, P_GRASS,P_GRASS,P_GRASS,P_GRASS,
                             P_WALL,P_WALL,P_WALL,P_WALL, P_WALL,P_WALL,P_WALL,P_WALL,
                             0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0),
    [MT_BLDG_DOOR  ] = BLK_OBJ(P_GRASS,P_WALL,
                               0x0A,0x4B,0x4B,0x0A, 0x4B,0x4B,0x4B,0x4B,
                               0x0B,0x0C,0x0A,0x0A, 0x1B,0x1C,0x1A,0x1A),
    [MT_BLDG_WALL_L] = BLK_OBJ(P_GRASS,P_WALL,
                               0x25,0x26,0x0A,0x22, 0x5C,0x17,0x17,0x17,
                               0x0F,0x22,0x0B,0x0C, 0x4E,0x1A,0x1B,0x1C),
    [MT_BLDG_WALL_R] = BLK_OBJ(P_GRASS,P_WALL,
                               0x0A,0x0A,0x28,0x29, 0x17,0x17,0x17,0x5D,
                               0x0A,0x0A,0x22,0x1F, 0x1A,0x1A,0x1A,0x4F),

    [MT_BORDER_R   ] = BLK_AUTO(0x2C,0x2C,0x2A,0x2B, 0x2C,0x2C,0x3A,0x3B, 0x2C,0x2C,0x2A,0x2B, 0x2C,0x2C,0x3A,0x3B),
    [MT_BORDER_L   ] = BLK_AUTO(0x2A,0x2B,0x2C,0x2C, 0x3A,0x3B,0x2C,0x2C, 0x2A,0x2B,0x2C,0x2C, 0x3A,0x3B,0x2C,0x2C),
    [MT_BORDER_TR  ] = BLK_AUTO(0x2C,0x2C,0x2A,0x2B, 0x2C,0x2C,0x3A,0x3B, 0x2A,0x2B,0x2A,0x2B, 0x3A,0x3B,0x3A,0x3B),
    [MT_BORDER_BL  ] = BLK_AUTO(0x2A,0x2B,0x2C,0x2C, 0x3A,0x3B,0x2C,0x2C, 0x2A,0x2B,0x2A,0x2B, 0x3A,0x3B,0x3A,0x3B),
    [MT_TREE_TOP   ] = BLK_AUTO(0x2C,0x2C,0x2C,0x2C, 0x2C,0x2C,0x2C,0x2C, 0x2A,0x2B,0x2A,0x2B, 0x3A,0x3B,0x3A,0x3B),
    [MT_SIGN       ] = BLK2(0x39,0x39,0x39,0x39, 0x39,0x39,0x39,0x39, 0x0E,0x0E,0x46,0x47, 0x55,0x55,0x56,0x57,
                            0,0,0,0, 0,0,0,0, O(0x0E),O(0x0E),O(0x46),O(0x47), O(0x55),O(0x55),O(0x56),O(0x57),
                            P_GRASS,P_GRASS,P_GRASS,P_GRASS, P_GRASS,P_GRASS,P_GRASS,P_GRASS,
                            P_GRASS,P_GRASS,P_GRASS,P_GRASS, P_GRASS,P_GRASS,P_GRASS,P_GRASS,
                            0,0,0,0, 0,0,0,0, P_WOOD,P_WOOD,P_WOOD,P_WOOD, P_WOOD,P_WOOD,P_WOOD,P_WOOD),

    [MT_ROUTE_B    ] = BLK(0x39,0x39,0x39,0x39, 0x39,0x39,0x39,0x39, 0x2A,0x2B,0x2A,0x2B, 0x3A,0x3B,0x3A,0x3B,
                           P_GRASS,P_GRASS,P_GRASS,P_GRASS, P_GRASS,P_GRASS,P_GRASS,P_GRASS,
                           P_TREE,P_TREE,P_TREE,P_TREE, P_TREE,P_TREE,P_TREE,P_TREE),
    [MT_ROUTE_BL   ] = BLK(0x14,0x14,0x14,0x36, 0x14,0x14,0x14,0x36, 0x14,0x14,0x14,0x36, 0x14,0x14,0x14,0x36,
                           P_WATER,P_WATER,P_WATER,P_WOOD, P_WATER,P_WATER,P_WATER,P_WOOD,
                           P_WATER,P_WATER,P_WATER,P_WOOD, P_WATER,P_WATER,P_WATER,P_WOOD),
    // Block 0x65 from gfx/blocksets/overworld.bst. Its left edge is the
    // rocky shoreline tile 0x32; 0x20 is a different horizontal transition
    // tile and causes the shoreline to appear stretched.
    [MT_ROUTE_BR   ] = BLK(0x32,0x14,0x14,0x14, 0x32,0x14,0x14,0x14, 0x32,0x14,0x14,0x14, 0x32,0x14,0x14,0x14,
                           P_WOOD,P_WATER,P_WATER,P_WATER, P_WOOD,P_WATER,P_WATER,P_WATER,
                           P_WOOD,P_WATER,P_WATER,P_WATER, P_WOOD,P_WATER,P_WATER,P_WATER),
    [MT_ROAD_H     ] = BLK2(0x2C,0x2C,0x2C,0x2C, 0x2C,0x03,0x2C,0x03, 0x03,0x2C,0x03,0x2C, 0x2C,0x2C,0x2C,0x2C,
                             0,0,0,0, 0,O(0x03),0,O(0x03), O(0x03),0,O(0x03),0, 0,0,0,0,
                             P_PATH,P_PATH,P_PATH,P_PATH, P_PATH,P_PATH,P_PATH,P_PATH,
                             P_PATH,P_PATH,P_PATH,P_PATH, P_PATH,P_PATH,P_PATH,P_PATH,
                             0,0,0,0, 0,P_FLOWER,0,P_FLOWER, P_FLOWER,0,P_FLOWER,0, 0,0,0,0),
    [MT_ROAD_CORNER] = BLK2(0x39,0x39,0x39,0x39, 0x39,0x39,0x39,0x39, 0x0E,0x0E,0x0E,0x0E, 0x55,0x55,0x55,0x55,
                             0,0,0,0, 0,0,0,0, O(0x0E),O(0x0E),O(0x0E),O(0x0E), O(0x55),O(0x55),O(0x55),O(0x55),
                             P_GRASS,P_GRASS,P_GRASS,P_GRASS, P_GRASS,P_GRASS,P_GRASS,P_GRASS,
                             P_GRASS,P_GRASS,P_GRASS,P_GRASS, P_GRASS,P_GRASS,P_GRASS,P_GRASS,
                             0,0,0,0, 0,0,0,0, P_WOOD,P_WOOD,P_WOOD,P_WOOD, P_WOOD,P_WOOD,P_WOOD,P_WOOD),
};
#undef PAL_ALL
#undef BLK_ALL
#undef BLK_AUTO
#undef BLK2
#undef BLK
#undef O
#undef OE
#undef OS
#undef OH
#undef P_WATER
#undef P_FLOWER
#undef P_SHORE
#undef P_WOOD
#undef P_TREE
#undef P_ROOF
#undef P_WALL
#undef P_PATH
#undef P_GRASS
#undef PAL_TILE

const Tileset g_tileset_overworld = {
    .tiles          = g_overworld_tiles,
    .tile_count     = 96,
    .palettes       = s_palettes,
    .palette_count  = 9,
    .metatiles      = s_metatiles,
    .metatile_count = 120,
};
