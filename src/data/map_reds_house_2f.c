// Red's House 2F — 4×4 blocks.
// Layout from refs/pokered/maps/RedsHouse2F.blk.
// Warp at (7,1) → RedsHouse1F warp index 2 (the upstairs warp on 1F).
#include "world.h"
#include "map_ids.h"

extern const Tileset g_tileset_house;

// P = passable, W = impassable
#define P(m) MAPCELL_MAKE((m), 0, 0)
#define W(m) MAPCELL_MAKE((m), 1, 0)

// Row 0: 0x10(stair top-L) 0x11(stair top-R) 0x05(window) 0x08(desk/NES)
// Row 1: 0x0F(floor)       0x0F(floor)        0x0F(floor)  0x0F(floor)
// Row 2: 0x0F(floor)       0x0D(desk/bed)     0x0F(floor)  0x0F(floor)
// Row 3: 0x0C(stairs down) 0x0F(floor)        0x0F(floor)  0x12(wardrobe)
static const MapCell s_cells[] = {
    W(0x10), W(0x11), W(0x05), W(0x08),
    P(0x0F), P(0x0F), P(0x0F), P(0x0F),
    P(0x0F), W(0x0D), P(0x0F), P(0x0F),
    P(0x0C), P(0x0F), P(0x0F), W(0x12),
};
#undef P
#undef W

static const MapLayout s_layout = {
    .width   = 4,
    .height  = 4,
    .tileset = &g_tileset_house,
    .cells   = s_cells,
};

// Warp to 1F down the staircase. Exactly at (7,1) to match 1F warp[2] position.
// This serves as both spawn point (when arriving from 1F) and trigger (stepping onto it).
static const WarpEvent s_warps[] = {
    { .x = 7, .y = 1, .dest_map = MAP_PLAYERS_HOUSE_1F, .dest_warp = 2 },
};

const MapHeader g_map_reds_house_2f = {
    .map_id     = MAP_PLAYERS_HOUSE_2F,
    .name       = "Red's House 2F",
    .layout     = &s_layout,
    .warps      = s_warps,
    .warp_count = 1,
    .npcs       = NULL,
    .npc_count  = 0,
    .script     = NULL,
    .music_id   = 0,
};
