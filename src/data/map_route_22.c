// Route 22 — 20x9 block layout west of Viridian City.
// Leads to the Pokemon League (Route 22 Gate). Features a rival encounter.
#include "world.h"
#include "audio.h"
#include "map_ids.h"
#include "overworld_palette_map.h"
#include "gfx_npcs.h"

extern const Tileset g_tileset_route22;
extern void script_route_22(void);

#define P(m) MAPCELL_MAKE((m), 0, 0)

// 20 blocks wide × 9 blocks tall — direct port from pokered Route22.blk.
// Uses the OVERWORLD tileset (same blockset as Pallet Town / Viridian City).
static const MapCell s_cells[20 * 9] = {
    // Row 0: building roof and border
    P(0x13), P(0x20), P(0x0D), P(0x0D), P(0x0D), P(0x0D), P(0x21), P(0x28), P(0x2C), P(0x2C), P(0x2C), P(0x2C), P(0x2C), P(0x2C), P(0x2B), P(0x57), P(0x57), P(0x57), P(0x2A), P(0x2C),
    // Row 1: building walls
    P(0x3B), P(0x68), P(0x7F), P(0x7F), P(0x7F), P(0x7F), P(0x69), P(0x24), P(0x57), P(0x57), P(0x57), P(0x57), P(0x57), P(0x57), P(0x25), P(0x07), P(0x2F), P(0x07), P(0x28), P(0x2C),
    // Row 2: water area and dense forest
    P(0x29), P(0x37), P(0x7D), P(0x7D), P(0x3A), P(0x7D), P(0x7E), P(0x13), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x24), P(0x57),
    // Row 3: path with gate building entrance
    P(0x29), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x13), P(0x07), P(0x07), P(0x07), P(0x1D), P(0x1E), P(0x3E), P(0x3B), P(0x2F), P(0x07), P(0x62), P(0x0A), P(0x0A),
    // Row 4: grass and gate lower portion
    P(0x29), P(0x07), P(0x07), P(0x07), P(0x07), P(0x2F), P(0x07), P(0x13), P(0x0B), P(0x0B), P(0x0B), P(0x65), P(0x64), P(0x28), P(0x29), P(0x0B), P(0x0B), P(0x4E), P(0x01), P(0x01),
    // Row 5: path, ledge, water, grass
    P(0x29), P(0x31), P(0x31), P(0x56), P(0x77), P(0x77), P(0x77), P(0x13), P(0x0B), P(0x0B), P(0x0B), P(0x55), P(0x55), P(0x28), P(0x29), P(0x0B), P(0x0B), P(0x4E), P(0x01), P(0x3E),
    // Row 6: trees and path
    P(0x29), P(0x07), P(0x07), P(0x07), P(0x07), P(0x07), P(0x07), P(0x07), P(0x07), P(0x07), P(0x07), P(0x07), P(0x07), P(0x24), P(0x25), P(0x07), P(0x2F), P(0x07), P(0x07), P(0x28),
    // Row 7: dense forest
    P(0x29), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x55), P(0x28),
    // Row 8: water bottom border
    P(0x2C), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x3F), P(0x2C),
};

static const MapLayout s_layout = {
    .width = 20,
    .height = 9,
    .tileset = &g_tileset_route22,
    .cells = s_cells,
};

static const WarpEvent s_warps[] = {
    // East connection to Viridian City (warp indices 8/9 in Viridian).
    { .x = 38, .y = 9, .dest_map = MAP_VIRIDIAN_CITY, .dest_warp = 8 },
    { .x = 38, .y = 10, .dest_map = MAP_VIRIDIAN_CITY, .dest_warp = 9 },
    // Route 22 Gate entrance (door at block 4,2 = subtile 8,5).
    { .x = 8, .y = 5, .dest_map = MAP_ROUTE_22_GATE, .dest_warp = 0 },
};

static const NpcDef s_npcs[] = {
    // Rival — hidden by default, shown by script when conditions met.
    // pokered places both toggleable Rival objects on this exact tile.
    { .x = 25, .y = 5, .sprite_tile = GFX_BLUE_TILE_BASE,
      .facing = DIR_LEFT, .flags = NPCF_HIDDEN,
      .script_id = 0, .movement = NPC_MOVE_STAY },
};

const MapHeader g_map_route_22 = {
    .map_id = MAP_ROUTE_22,
    .name = "Route 22",
    .layout = &s_layout,
    .warps = s_warps,
    .warp_count = 3,
    .npcs = s_npcs,
    .npc_count = 1,
    .script = script_route_22,
    .music_id = AUDIO_MUSIC_ROUTES_1,
    .roof_palette = &g_roof_pallet,
};

#undef P
