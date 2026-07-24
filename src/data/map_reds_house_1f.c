// Red's House 1F — 4×4 blocks.
// Layout from refs/pokered/maps/RedsHouse1F.blk.
// Warps: front door → Pallet Town, stairs → 2F.
#include "world.h"
#include "map_ids.h"
#include "gfx_npcs.h"
#include "audio.h"

extern const Tileset g_tileset_house;

#define P(m) MAPCELL_MAKE((m), 0, 0)
#define W(m) MAPCELL_MAKE((m), 1, 0)

// RedsHouse1F.blk: 04 09 05 07 / 0F 0F 0F 0F / 0F 01 02 0F / 0F 0B 0F 0F
// Row 0: 0x04(TV cabinet) 0x09(cabinet) 0x05(window) 0x07(window+plant)
// Row 1: 0x0F(floor)      0x0F(floor)   0x0F(floor)  0x0F(floor)
// Row 2: 0x0F(floor)      0x01(plant)   0x02(plant-R) 0x0F(floor)
// Row 3: 0x0F(floor)      0x0B(ledge)   0x0F(floor)   0x0F(floor)
// The front door is at the bottom edge (y=7 = map exit tiles).
static const MapCell s_cells[] = {
    W(0x04), W(0x09), W(0x05), W(0x07),
    P(0x0F), P(0x0F), P(0x0F), P(0x0F),
    P(0x0F), W(0x01), W(0x02), P(0x0F),
    P(0x0F), P(0x0B), P(0x0F), P(0x0F),
};
#undef P
#undef W

static const MapLayout s_layout = {
    .width   = 4,
    .height  = 4,
    .tileset = &g_tileset_house,
    .cells   = s_cells,
};

// Warp indices (0-based):
//   0: front door left  (2,7) → Pallet Town warp 0
//   1: front door right (3,7) → Pallet Town warp 0
//   2: stairs to 2F    (7,1) → RedsHouse2F warp 0 (collision warp; also spawn point when arriving from 2F)
//
// Pokered: warp_event 2,7,LAST_MAP,1 and warp_event 3,7,LAST_MAP,1 (dest_warp 0 = house entrance)
//          warp_event 7,1,REDS_HOUSE_2F,1 → dest_warp 0 = 2F warp[0] at (7,1)
static const WarpEvent s_warps[] = {
    // 0: front door left
    { .x = 2, .y = 7, .dest_map = MAP_PALLET_TOWN, .dest_warp = 0 },
    // 1: front door right
    { .x = 3, .y = 7, .dest_map = MAP_PALLET_TOWN, .dest_warp = 0 },
    // 2: stairs to 2F
    { .x = 7, .y = 1, .dest_map = MAP_PLAYERS_HOUSE_2F, .dest_warp = 0 },
};

const MapHeader g_map_reds_house_1f = {
    .map_id     = MAP_PLAYERS_HOUSE_1F,
    .name       = "Red's House 1F",
    .layout     = &s_layout,
    .warps      = s_warps,
    .warp_count = 3,
    .npcs       = (const NpcDef[]) {
        { .x = 5, .y = 4, .sprite_tile = GFX_MOM_TILE_BASE,
          .facing = DIR_LEFT, .flags = 0, .script_id = 8,
          .movement = NPC_MOVE_STAY },
    },
    .npc_count  = 1,
    .script     = NULL,
    .music_id   = AUDIO_MUSIC_PALLET_TOWN,
};
