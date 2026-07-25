// Viridian Gym — 10×9 block layout from pokered/maps/ViridianGym.blk.
// Uses the existing GYM tileset (shared with Oak's Lab).
// The gym is locked until the player has seven badges; for now a
// placeholder NPC delivers the locked-door message.
#include "world.h"
#include "gfx_npcs.h"
#include "gfx_npcs_extra.h"
#include "audio.h"
#include "map_ids.h"

extern const Tileset g_tileset_gym;

#define P(m) MAPCELL_MAKE((m), 0, 0)

// pokered ViridianGym.blk (10×9 = 90 bytes)
static const MapCell s_cells[10 * 9] = {
    P(0x49),P(0x01),P(0x4A),P(0x01),P(0x01),P(0x5F),P(0x01),P(0x01),P(0x01),P(0x5D),
    P(0x4B),P(0x05),P(0x51),P(0x64),P(0x56),P(0x63),P(0x46),P(0x46),P(0x5E),P(0x5C),
    P(0x52),P(0x46),P(0x4E),P(0x53),P(0x54),P(0x55),P(0x05),P(0x05),P(0x4C),P(0x05),
    P(0x61),P(0x05),P(0x60),P(0x4C),P(0x55),P(0x48),P(0x48),P(0x5B),P(0x4C),P(0x05),
    P(0x40),P(0x44),P(0x4C),P(0x4C),P(0x05),P(0x05),P(0x05),P(0x4D),P(0x4C),P(0x05),
    P(0x05),P(0x05),P(0x4D),P(0x4C),P(0x05),P(0x05),P(0x2C),P(0x55),P(0x4C),P(0x05),
    P(0x05),P(0x2C),P(0x58),P(0x48),P(0x48),P(0x48),P(0x5A),P(0x05),P(0x62),P(0x05),
    P(0x43),P(0x45),P(0x57),P(0x46),P(0x46),P(0x46),P(0x59),P(0x32),P(0x05),P(0x31),
    P(0x40),P(0x05),P(0x05),P(0x41),P(0x05),P(0x05),P(0x42),P(0x05),P(0x04),P(0x05),
};

static const MapLayout s_layout = {
    .width = 10, .height = 9, .tileset = &g_tileset_gym, .cells = s_cells,
};

// Warp 0: main entrance (bottom center), returns to Viridian City warp 5
static const WarpEvent s_warps[] = {
    { .x = 16, .y = 17, .dest_map = WARP_LAST_MAP, .dest_warp = 6 },
    { .x = 17, .y = 17, .dest_map = WARP_LAST_MAP, .dest_warp = 6 },
};

// Placeholder NPC: gym guide who tells you the gym is locked
static const NpcDef s_npcs[] = {
    { .x = 15, .y = 15, .sprite_tile = GFX_BLUE_TILE_BASE, .facing = DIR_DOWN, .script_id = 35 },
};

const MapHeader g_map_viridian_gym = {
    .map_id = MAP_VIRIDIAN_GYM, .name = "Viridian Gym", .layout = &s_layout,
    .warps = s_warps, .warp_count = ARRAY_COUNT(s_warps),
    .npcs = s_npcs, .npc_count = ARRAY_COUNT(s_npcs),
    .script = NULL, .music_id = AUDIO_MUSIC_VIRIDIAN_CITY,
};

#undef P
