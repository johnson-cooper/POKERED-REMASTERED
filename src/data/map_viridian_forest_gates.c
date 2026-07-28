#include "world.h"
#include "gfx_npcs.h"
#include "map_ids.h"
#include "audio.h"

// Reference tileset: FOREST_GATE. pokered aliases its graphics/blockset to
// GATE, but keeping the identity explicit prevents future map imports from
// accidentally selecting the forest overworld tileset.
extern const Tileset g_tileset_forest_gate;

#define P(m) MAPCELL_MAKE((m), 0, 0)

static const MapCell s_gate_cells[5 * 4] = {
    P(0x5E),P(0x5C),P(0x73),P(0x5C),P(0x5F),
    P(0x3A),P(0x00),P(0x00),P(0x62),P(0x63),
    P(0x3A),P(0x00),P(0x00),P(0x60),P(0x64),
    P(0x3A),P(0x00),P(0x0B),P(0x61),P(0x65),
};

static const MapLayout s_gate_layout = {
    .width = 5, .height = 4, .tileset = &g_tileset_forest_gate,
    .cells = s_gate_cells,
};

static const WarpEvent s_north_warps[] = {
    { .x=4, .y=0, .dest_map=WARP_LAST_MAP, .dest_warp=3 },
    { .x=5, .y=0, .dest_map=WARP_LAST_MAP, .dest_warp=3 },
    { .x=4, .y=7, .dest_map=MAP_VIRIDIAN_FOREST, .dest_warp=0 },
    { .x=5, .y=7, .dest_map=MAP_VIRIDIAN_FOREST, .dest_warp=0 },
};

static const WarpEvent s_south_warps[] = {
    { .x=5, .y=0, .dest_map=MAP_VIRIDIAN_FOREST, .dest_warp=2 },
    { .x=4, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=4 },
    { .x=5, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=4 },
};

static const NpcDef s_north_npcs[] = {
    { .x=3, .y=2, .sprite_tile=GFX_YOUNGSTER_TILE_BASE,
      .facing=DIR_DOWN, .movement=NPC_MOVE_STAY,
      .text="Many POKeMON live only in\\nforests and caves.\\fYou need to look\\neverywhere to get\\ndifferent kinds!" },
    { .x=2, .y=5, .sprite_tile=GFX_YOUNGSTER_TILE_BASE,
      .facing=DIR_DOWN, .movement=NPC_MOVE_STAY,
      .text="Bushes can be cut down\\nby a special POKeMON move." },
};

static const NpcDef s_south_npcs[] = {
    { .x=8, .y=4, .sprite_tile=GFX_GIRL_TILE_BASE,
      .facing=DIR_LEFT, .movement=NPC_MOVE_STAY,
      .text="Are you going to VIRIDIAN\\nFOREST? Be careful, it's\\na natural maze!" },
    { .x=2, .y=4, .sprite_tile=GFX_GIRL_TILE_BASE,
      .facing=DIR_UP, .movement=NPC_MOVE_UP_DOWN,
      .text="RATTATA may be small,\\nbut its bite is wicked!" },
};

const MapHeader g_map_viridian_forest_north_gate = {
    .map_id=MAP_VIRIDIAN_FOREST_NORTH_GATE, .name="Viridian Forest North Gate",
    .layout=&s_gate_layout, .warps=s_north_warps,
    .warp_count=ARRAY_COUNT(s_north_warps), .npcs=s_north_npcs,
    .npc_count=ARRAY_COUNT(s_north_npcs), .script=NULL,
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY, .roof_palette=NULL,
};

const MapHeader g_map_viridian_forest_south_gate = {
    .map_id=MAP_VIRIDIAN_FOREST_SOUTH_GATE, .name="Viridian Forest South Gate",
    .layout=&s_gate_layout, .warps=s_south_warps,
    .warp_count=ARRAY_COUNT(s_south_warps), .npcs=s_south_npcs,
    .npc_count=ARRAY_COUNT(s_south_npcs), .script=NULL,
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY, .roof_palette=NULL,
};

#undef P
