// Rival's House (BluesHouse) — 4×4 blocks from pokered/maps/BluesHouse.blk.
#include "world.h"
#include "map_ids.h"
#include "gfx_npcs.h"

extern const Tileset g_tileset_house_general;

#define P(m) MAPCELL_MAKE((m), 0, 0)
#define W(m) MAPCELL_MAKE((m), 1, 0)

// BluesHouse.blk:
//   04 0E 05 09
//   0F 01 02 0F
//   0F 0C 0D 0F
//   06 0B 0F 07
static const MapCell s_cells[] = {
    W(0x04), W(0x0E), W(0x05), W(0x09),
    P(0x0F), W(0x01), W(0x02), P(0x0F),
    P(0x0F), P(0x0C), W(0x0D), P(0x0F),
    P(0x06), P(0x0B), P(0x0F), W(0x07),
};
#undef P
#undef W

static const MapLayout s_layout = {
    .width = 4,
    .height = 4,
    .tileset = &g_tileset_house_general,
    .cells = s_cells,
};

// Pokered's two front-door tiles return to Pallet Town's rival-house warp.
static const WarpEvent s_warps[] = {
    { .x = 2, .y = 7, .dest_map = WARP_LAST_MAP, .dest_warp = 1 },
    { .x = 3, .y = 7, .dest_map = WARP_LAST_MAP, .dest_warp = 1 },
};

static const NpcDef s_npcs[] = {
    { .x = 2, .y = 3, .sprite_tile = GFX_DAISY_TILE_BASE,
      .facing = DIR_RIGHT, .flags = 0, .script_id = 5,
      .movement = NPC_MOVE_STAY },
    { .x = 6, .y = 4, .sprite_tile = GFX_DAISY_TILE_BASE,
      .facing = DIR_UP, .flags = 0, .script_id = 6,
      .movement = NPC_MOVE_UP_DOWN },
    // Pokered's BLUESHOUSE_TOWN_MAP background object.
    { .x = 3, .y = 3, .sprite_tile = GFX_POKEDEX_TILE_BASE,
      .facing = DIR_DOWN, .flags = 0, .script_id = 7,
      .movement = NPC_MOVE_STAY },
};

const MapHeader g_map_rivals_house = {
    .map_id = MAP_RIVALS_HOUSE,
    .name = "Rival's House",
    .layout = &s_layout,
    .warps = s_warps,
    .warp_count = 2,
    .npcs = s_npcs,
    .npc_count = ARRAY_COUNT(s_npcs),
    .script = NULL,
    .music_id = 0,
};
