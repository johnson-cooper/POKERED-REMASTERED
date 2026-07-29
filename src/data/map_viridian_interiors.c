// Viridian's Mart, Pokécenter, and houses. Block order comes from pokered
// map .blk files; the existing interior tileset supplies the shared
// floor/wall vocabulary.
#include "world.h"
#include "gfx_npcs.h"
#include "gfx_npcs_extra.h"
#include "map_ids.h"
#include "audio.h"

extern const Tileset g_tileset_house_general;
extern const Tileset g_tileset_pokecenter;
extern void script_viridian_mart(void);

#define P(m) MAPCELL_MAKE((m), 0, 0)

// ── Viridian Mart (4×4) ─────────────────────────────────────────────────────
// Exact ViridianMart.blk layout using pokered's shared MART/POKECENTER
// tileset. In pokered, Mart_GFX/Mart_Block alias Pokecenter_GFX/Block.
static const MapCell s_mart_cells[] = {
    P(0x12),P(0x13),P(0x13),P(0x09),
    P(0x16),P(0x0F),P(0x14),P(0x14),
    P(0x18),P(0x19),P(0x15),P(0x15),
    P(0x17),P(0x1A),P(0x0B),P(0x0F),
};

// ── Viridian School House (4×4) ─────────────────────────────────────────────
// pokered ViridianSchoolHouse.blk: 05 12 13 09  0F 14 15 0F  0F 18 19 0F  06 0B 0F 07
static const MapCell s_school_cells[] = {
    P(0x05),P(0x12),P(0x13),P(0x09),
    P(0x0F),P(0x14),P(0x15),P(0x0F),
    P(0x0F),P(0x18),P(0x19),P(0x0F),
    P(0x06),P(0x0B),P(0x0F),P(0x07),
};

// ── Viridian Nickname House (4×4) ───────────────────────────────────────────
// pokered ViridianNicknameHouse.blk: 04 0E 05 09  0F 01 02 0F  0F 0C 0D 0F  06 0B 0F 07
static const MapCell s_nickname_cells[] = {
    P(0x04),P(0x0E),P(0x05),P(0x09),
    P(0x0F),P(0x01),P(0x02),P(0x0F),
    P(0x0F),P(0x0C),P(0x0D),P(0x0F),
    P(0x06),P(0x0B),P(0x0F),P(0x07),
};

// ── Viridian Pokécenter (7×4) ───────────────────────────────────────────────
// Exact ViridianPokecenter.blk layout using the reference POKECENTER tileset.
static const MapCell s_center_cells[] = {
    P(0x20),P(0x10),P(0x01),P(0x02),P(0x0C),P(0x0D),P(0x0D),P(0x21),
    P(0x04),P(0x05),P(0x07),P(0x07),P(0x22),P(0x23),P(0x08),P(0x0F),
    P(0x0F),P(0x0F),P(0x0F),P(0x0F),P(0x1B),P(0x0E),P(0x0A),P(0x0B),
    P(0x0E),P(0x0F),P(0x0F),P(0x0F),P(0x0F),P(0x1B),P(0x0E),P(0x0F),
};

static const MapLayout s_mart_layout     = { .width=4, .height=4, .tileset=&g_tileset_pokecenter, .cells=s_mart_cells };
static const MapLayout s_school_layout   = { .width=4, .height=4, .tileset=&g_tileset_house_general, .cells=s_school_cells };
static const MapLayout s_nickname_layout = { .width=4, .height=4, .tileset=&g_tileset_house_general, .cells=s_nickname_cells };
static const MapLayout s_center_layout   = { .width=7, .height=4, .tileset=&g_tileset_pokecenter, .cells=s_center_cells };

// ── Warps ────────────────────────────────────────────────────────────────────
// pokered dest_warp is 1-indexed; this project is 0-indexed.
static const WarpEvent s_mart_warps[] = {
    { .x=3, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=3 },
    { .x=4, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=3 },
};
static const WarpEvent s_center_warps[] = {
    // A blackout recovery always returns to Viridian City, never to the map
    // that happened to be active before the player entered the center.
    { .x=3, .y=7, .dest_map=MAP_VIRIDIAN_CITY, .dest_warp=2 },
    { .x=4, .y=7, .dest_map=MAP_VIRIDIAN_CITY, .dest_warp=2 },
};
static const WarpEvent s_school_warps[] = {
    { .x=2, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=4 },
    { .x=3, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=4 },
};
static const WarpEvent s_nickname_warps[] = {
    { .x=2, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=5 },
    { .x=3, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=5 },
};

// ── NPCs ─────────────────────────────────────────────────────────────────────
// Positions match pokered object_event coordinates.

// Mart: Clerk (0,5) RIGHT, Youngster (5,5) UP_DOWN, Cooltrainer_M (3,3) STAY
static const NpcDef s_mart_npcs[] = {
    { .x=0, .y=5, .sprite_tile=GFX_SCIENTIST_TILE_BASE, .facing=DIR_RIGHT, .script_id=22 },
    { .x=5, .y=5, .sprite_tile=GFX_BLUE_TILE_BASE,      .facing=DIR_UP,    .script_id=23, .movement=NPC_MOVE_UP_DOWN },
    { .x=3, .y=3, .sprite_tile=GFX_BLUE_TILE_BASE,      .facing=DIR_DOWN,  .script_id=24 },
};

// Pokécenter: Nurse (3,1) DOWN, Gentleman (10,5) UP_DOWN,
//             Cooltrainer_M (5,3) STAY, Link Receptionist (11,2) DOWN,
//             PC (13,5) — the east-side 0x08 computer tile provides the visual;
//             NPCF_NO_SPRITE keeps the trigger invisible.
static const NpcDef s_center_npcs[] = {
    { .x=3,  .y=1, .sprite_tile=GFX_GIRL_TILE_BASE,      .facing=DIR_DOWN, .script_id=25 },
    { .x=10, .y=5, .sprite_tile=GFX_FISHER_TILE_BASE,    .facing=DIR_DOWN, .script_id=26, .movement=NPC_MOVE_UP_DOWN },
    { .x=5,  .y=3, .sprite_tile=GFX_BLUE_TILE_BASE,      .facing=DIR_DOWN, .script_id=27 },
    { .x=11, .y=2, .sprite_tile=GFX_SCIENTIST_TILE_BASE, .facing=DIR_DOWN, .script_id=28 },
    { .x=13, .y=5, .sprite_tile=0,                       .facing=DIR_DOWN, .script_id=36,
      .movement=NPC_MOVE_STAY, .flags=NPCF_NO_SPRITE },
};

// School: Brunette Girl (3,5) UP, Cooltrainer_F (4,1) DOWN
static const NpcDef s_school_npcs[] = {
    { .x=3, .y=5, .sprite_tile=GFX_GIRL_TILE_BASE, .facing=DIR_UP,   .script_id=29 },
    { .x=4, .y=1, .sprite_tile=GFX_GIRL_TILE_BASE, .facing=DIR_DOWN, .script_id=30 },
};

// Nickname House: Balding Guy (5,3) STAY, Little Girl (1,4) UP_DOWN,
//                 Spearow (5,5) LEFT_RIGHT
static const NpcDef s_nickname_npcs[] = {
    { .x=5, .y=3, .sprite_tile=GFX_FISHER_TILE_BASE, .facing=DIR_DOWN,  .script_id=31 },
    { .x=1, .y=4, .sprite_tile=GFX_GIRL_TILE_BASE,   .facing=DIR_DOWN,  .script_id=32, .movement=NPC_MOVE_UP_DOWN },
    { .x=5, .y=5, .sprite_tile=GFX_GIRL_TILE_BASE,   .facing=DIR_LEFT,  .script_id=33, .movement=NPC_MOVE_LEFT_RIGHT },
};

// ── Map Headers ──────────────────────────────────────────────────────────────

const MapHeader g_map_viridian_mart = {
    .map_id=MAP_VIRIDIAN_MART, .name="Viridian Mart", .layout=&s_mart_layout,
    .warps=s_mart_warps, .warp_count=ARRAY_COUNT(s_mart_warps),
    .npcs=s_mart_npcs, .npc_count=ARRAY_COUNT(s_mart_npcs),
    .script=script_viridian_mart, .music_id=AUDIO_MUSIC_VIRIDIAN_CITY,
};
const MapHeader g_map_viridian_pokecenter = {
    .map_id=MAP_VIRIDIAN_POKECENTER, .name="Viridian Pokemon Center", .layout=&s_center_layout,
    .warps=s_center_warps, .warp_count=ARRAY_COUNT(s_center_warps),
    .npcs=s_center_npcs, .npc_count=ARRAY_COUNT(s_center_npcs), .script=NULL,
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY,
};
const MapHeader g_map_viridian_school_house = {
    .map_id=MAP_VIRIDIAN_SCHOOL_HOUSE, .name="Viridian School", .layout=&s_school_layout,
    .warps=s_school_warps, .warp_count=ARRAY_COUNT(s_school_warps),
    .npcs=s_school_npcs, .npc_count=ARRAY_COUNT(s_school_npcs), .script=NULL,
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY,
};
const MapHeader g_map_viridian_nickname_house = {
    .map_id=MAP_VIRIDIAN_NICKNAME_HOUSE, .name="Viridian Nickname House", .layout=&s_nickname_layout,
    .warps=s_nickname_warps, .warp_count=ARRAY_COUNT(s_nickname_warps),
    .npcs=s_nickname_npcs, .npc_count=ARRAY_COUNT(s_nickname_npcs), .script=NULL,
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY,
};

#undef P
