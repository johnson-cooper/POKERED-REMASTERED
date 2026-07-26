// Route 22 Gate — 5x4 block indoor map, GATE tileset.
// Blocks the player from proceeding to the Pokemon League without badges.
#include "world.h"
#include "gfx_npcs.h"
#include "map_ids.h"
#include "audio.h"

extern const Tileset g_tileset_house_general;

#define P(m) MAPCELL_MAKE((m), 0, 0)

static const MapCell s_gate_cells[] = {
    P(0x04),P(0x0E),P(0x05),P(0x05),P(0x09),
    P(0x0F),P(0x01),P(0x02),P(0x0F),P(0x0F),
    P(0x0F),P(0x0F),P(0x0F),P(0x0F),P(0x0F),
    P(0x06),P(0x0B),P(0x0F),P(0x0B),P(0x07),
};

static const MapLayout s_gate_layout = {
    .width = 5, .height = 4,
    .tileset = &g_tileset_house_general,
    .cells = s_gate_cells,
};

static const WarpEvent s_gate_warps[] = {
    { .x = 2, .y = 7, .dest_map = WARP_LAST_MAP, .dest_warp = 0 },
    { .x = 3, .y = 7, .dest_map = WARP_LAST_MAP, .dest_warp = 0 },
    { .x = 7, .y = 7, .dest_map = WARP_LAST_MAP, .dest_warp = 0 },
    { .x = 8, .y = 7, .dest_map = WARP_LAST_MAP, .dest_warp = 0 },
};

static const NpcDef s_gate_npcs[] = {
    { .x = 4, .y = 4, .sprite_tile = GFX_YOUNGSTER_TILE_BASE,
      .facing = DIR_DOWN, .script_id = 39, .movement = NPC_MOVE_STAY },
};

const MapHeader g_map_route_22_gate = {
    .map_id = MAP_ROUTE_22_GATE,
    .name = "Route 22 Gate",
    .layout = &s_gate_layout,
    .warps = s_gate_warps,
    .warp_count = 4,
    .npcs = s_gate_npcs,
    .npc_count = 1,
    .script = NULL,
    .music_id = AUDIO_MUSIC_VIRIDIAN_CITY,
    .roof_palette = NULL,
};

#undef P
