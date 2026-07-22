#include "world.h"
#include "map_ids.h"
#include "metatile_ids.h"

extern const Tileset g_tileset_overworld;

#define P(m) MAPCELL_MAKE((m), 0, 0)

// A playable Route 1 transition map. The overworld art is already shared with
// Pallet Town; the centered path keeps the opening sequence usable while the
// remainder of Route 1 is ported.
static const MapCell s_cells[10 * 18] = {
    [0 ... (10 * 18 - 1)] = P(MT_GRASS),
    [0 ... 9] = P(MT_TREE_TOP),
    [170 ... 179] = P(MT_ROUTE_B),
};

static const MapLayout s_layout = {
    .width = 10,
    .height = 18,
    .tileset = &g_tileset_overworld,
    .cells = s_cells,
};

static const WarpEvent s_warps[] = {
    { .x = 8, .y = 35, .dest_map = WARP_LAST_MAP, .dest_warp = 3 },
};

const MapHeader g_map_route_1 = {
    .map_id = MAP_ROUTE_1,
    .name = "Route 1",
    .layout = &s_layout,
    .warps = s_warps,
    .warp_count = 1,
    .npcs = NULL,
    .npc_count = 0,
    .script = NULL,
    .music_id = 0,
};

#undef P
