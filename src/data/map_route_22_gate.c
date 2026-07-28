// Route 22 Gate — 5x4 block indoor map, GATE tileset.
// Blocks the player from proceeding to the Pokemon League without badges.
#include "world.h"
#include "gfx_npcs.h"
#include "map_ids.h"
#include "audio.h"

extern const Tileset g_tileset_gate;

#define P(m) MAPCELL_MAKE((m), 0, 0)

static const MapCell s_gate_cells[] = {
    // Route22Gate.blk from pokered.
    P(0x03),P(0x2E),P(0x28),P(0x2F),P(0x03),
    P(0x00),P(0x00),P(0x04),P(0x00),P(0x00),
    P(0x00),P(0x00),P(0x04),P(0x00),P(0x00),
    P(0x00),P(0x2C),P(0x04),P(0x2D),P(0x00),
};

static const MapLayout s_gate_layout = {
    .width = 5, .height = 4,
    .tileset = &g_tileset_gate,
    .cells = s_gate_cells,
};

static const WarpEvent s_gate_warps[] = {
    // Route 22's gate-entry warp is index 4 in this project because the
    // connection warps to Viridian City are listed first.
    { .x = 4, .y = 7, .dest_map = WARP_LAST_MAP, .dest_warp = 4 },
    { .x = 5, .y = 7, .dest_map = WARP_LAST_MAP, .dest_warp = 4 },
    { .x = 4, .y = 0, .dest_map = WARP_LAST_MAP, .dest_warp = 0 },
    { .x = 5, .y = 0, .dest_map = WARP_LAST_MAP, .dest_warp = 1 },
};

static const NpcDef s_gate_npcs[] = {
    { .x = 6, .y = 2, .sprite_tile = GFX_YOUNGSTER_TILE_BASE,
      .facing = DIR_LEFT, .script_id = 39, .movement = NPC_MOVE_STAY },
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
