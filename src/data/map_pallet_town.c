// Pallet Town — 10×9 blocks, ported directly from pokered/maps/PalletTown.blk.
// Block IDs and layout match the original game exactly.
#include "world.h"
#include "gfx_npcs.h"
#include "map_ids.h"
#include "metatile_ids.h"

extern const Tileset g_tileset_overworld;

// P = passable (collision=0), W = impassable wall (collision=1)
#define P(m) MAPCELL_MAKE((m), 0, 0)
#define W(m) MAPCELL_MAKE((m), 1, 0)

// Pokered PalletTown.blk layout (hex block IDs):
//   52 4F 52 52 4F 0B 50 52 52 50
//   4E 01 38 39 01 01 38 39 01 4D
//   4E 08 3C 3D 01 08 3C 3D 01 4D
//   4E 01 01 01 01 01 01 01 01 4D
//   4E 01 77 56 01 0C 0D 0E 01 4D
//   4E 01 74 74 01 10 3A 00 01 4D
//   4E 01 01 01 01 77 56 77 31 4D
//   4E 0A 1D 1E 31 74 74 0A 31 4D
//   50 0A 65 64 61 61 61 61 61 4F
static const MapCell s_cells[] = {
//   col 0              1              2              3              4              5              6              7              8              9
    W(MT_TREE_TOP  ), W(MT_BORDER_TR), W(MT_TREE_TOP  ), W(MT_TREE_TOP  ), W(MT_BORDER_TR), W(MT_BORDER_TL), W(MT_BORDER_BL), W(MT_TREE_TOP  ), W(MT_TREE_TOP  ), W(MT_BORDER_BL), // row 0
    W(MT_BORDER_L  ), P(MT_GRASS     ), W(MT_BLDG_TOP_L), W(MT_BLDG_TOP_R), P(MT_GRASS     ), P(MT_GRASS     ), W(MT_BLDG_TOP_L), W(MT_BLDG_TOP_R), P(MT_GRASS     ), W(MT_BORDER_R  ), // row 1
    W(MT_BORDER_L  ), W(MT_TREE_CORNER),W(MT_BLDG_WALL_L),W(MT_BLDG_WALL_R),P(MT_GRASS     ), W(MT_TREE_CORNER),W(MT_BLDG_WALL_L),W(MT_BLDG_WALL_R),P(MT_GRASS     ), W(MT_BORDER_R  ), // row 2
    W(MT_BORDER_L  ), P(MT_GRASS     ), P(MT_GRASS     ), P(MT_GRASS     ), P(MT_GRASS     ), P(MT_GRASS     ), P(MT_GRASS     ), P(MT_GRASS     ), P(MT_GRASS     ), W(MT_BORDER_R  ), // row 3
    W(MT_BORDER_L  ), P(MT_GRASS     ), P(MT_ROAD_CORNER),W(MT_SIGN      ), P(MT_GRASS     ), W(MT_BLDG_LEFT ), W(MT_BLDG_MID  ), W(MT_BLDG_RIGHT), P(MT_GRASS     ), W(MT_BORDER_R  ), // row 4
    W(MT_BORDER_L  ), P(MT_GRASS     ), P(MT_ROAD_H    ), P(MT_ROAD_H    ), P(MT_GRASS     ), P(MT_DIRT      ), P(MT_BLDG_DOOR ), W(MT_GRASS_STEP ), P(MT_GRASS     ), W(MT_BORDER_R  ), // row 5
    W(MT_BORDER_L  ), P(MT_GRASS     ), P(MT_GRASS     ), P(MT_GRASS     ), P(MT_GRASS     ), P(MT_ROAD_CORNER),W(MT_SIGN      ), P(MT_ROAD_CORNER), P(MT_PATH_CORNER),W(MT_BORDER_R  ), // row 6
    W(MT_BORDER_L  ), P(MT_PATH_WALK ), P(MT_WALKWAY_L ), P(MT_WALKWAY_R ), P(MT_PATH_CORNER),P(MT_ROAD_H    ), P(MT_ROAD_H    ), P(MT_PATH_WALK ), P(MT_PATH_CORNER),W(MT_BORDER_R  ), // row 7
    W(MT_BORDER_BL ), P(MT_PATH_WALK ), P(MT_ROUTE_BR  ), P(MT_ROUTE_BL  ), P(MT_ROUTE_B   ), P(MT_ROUTE_B   ), P(MT_ROUTE_B   ), P(MT_ROUTE_B   ), P(MT_ROUTE_B   ), W(MT_BORDER_TR ), // row 8
};
#undef P
#undef W

static const MapLayout s_pallet_layout = {
    .width   = 10,
    .height  = 9,
    .tileset = &g_tileset_overworld,
    .cells   = s_cells,
};

// Pokered PalletTown_Object warps (pokered 1-indexed → our 0-indexed dest_warp):
//   warp_event  5,  5, REDS_HOUSE_1F, 1 → [0] collision warp (player walks north into house wall)
//   warp_event 13,  5, BLUES_HOUSE,   1 → [1] not implemented
//   warp_event 12, 11, OAKS_LAB,      2 → [2] step-on warp (passable door tile; dest_warp=2-1=1)
// Red's House 1F exits: LAST_MAP dest_warp=0 → lands here at warp[0]=(5,5)
// OaksLab exits:        LAST_MAP dest_warp=2 → lands here at warp[2]=(12,11)
static const WarpEvent s_warps[] = {
    // North connection to Route 1. The opening script intercepts this before
    // Oak is followed; afterward it behaves as the normal town exit.
    // [0] Red's House 1F entrance — collision warp at base of wall (x=5, y=5 = wall block bottom)
    { .x =  5, .y =  5, .dest_map = MAP_PLAYERS_HOUSE_1F, .dest_warp = 0 },
    // [1] Rival's house entrance.
    { .x = 13, .y =  5, .dest_map = MAP_RIVALS_HOUSE,      .dest_warp = 0 },
    // [2] Oak's Lab entrance — left door subtile
    { .x = 12, .y = 11, .dest_map = MAP_OAKS_LAB,          .dest_warp = 1 },
    // [3] Return point used by Route 1's south connection (dest_warp=3 must stay here).
    { .x = 8,  .y = 0,  .dest_map = MAP_ROUTE_1,           .dest_warp = 0 },
    // [4] Right subtile of the Oak's Lab door — mirrors [2] so either tile warps in.
    { .x = 13, .y = 11, .dest_map = MAP_OAKS_LAB,          .dest_warp = 1 },
};

extern void script_pallet_town(void);

// Oak begins just south of the north exit and is revealed by the opening
// Pallet Town script. The other two NPCs match the original map placement.
static const NpcDef s_npcs[] = {
    { .x = 8,  .y = 5,  .sprite_tile = GFX_OAK_TILE_BASE, .facing = DIR_UP,
      .flags = NPCF_HIDDEN, .script_id = 0 },
    { .x = 3,  .y = 8,  .sprite_tile = GFX_GIRL_TILE_BASE, .facing = DIR_DOWN,
      .flags = 0, .script_id = 3, .movement = NPC_MOVE_WALK_ANY },
    { .x = 11, .y = 14, .sprite_tile = GFX_FISHER_TILE_BASE, .facing = DIR_DOWN,
      .flags = 0, .script_id = 4, .movement = NPC_MOVE_WALK_ANY },
};

const MapHeader g_map_pallet_town = {
    .map_id     = MAP_PALLET_TOWN,
    .name       = "Pallet Town",
    .layout     = &s_pallet_layout,
    .warps      = s_warps,
    .warp_count = 5,
    .npcs       = s_npcs,
    .npc_count  = ARRAY_COUNT(s_npcs),
    .script     = script_pallet_town,
    .music_id   = 0,
};
