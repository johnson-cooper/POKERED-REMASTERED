// Oak's Lab (OaksLab) — 5×6 blocks, ported from pokered/maps/OaksLab.blk.
// Uses the DOJO tileset (Gym_GFX + Gym_Block in pokered).
// Warps lead back to Pallet Town warp[2] = (12,11) = lab entrance.
#include "world.h"
#include "map_ids.h"
#include "gfx_pokeball.h"
#include "gfx_npcs.h"

extern const Tileset g_tileset_gym;

#define P(m) MAPCELL_MAKE((m), 0, 0)
#define W(m) MAPCELL_MAKE((m), 1, 0)

// OaksLab.blk (5×6, all hex):
//   Row 0: 65 66 67 68 68   (lab equip top-L, equip top-R, bookshelf, counter, counter)
//   Row 1: 6B 6B 05 69 6A   (wall, wall, floor, equip-L, equip-R)
//   Row 2: 05 05 05 6D 6E   (floor×3, wall variant, wall corner)
//   Row 3: 68 68 05 68 68   (counter×2, floor, counter×2 — pokeball tables)
//   Row 4: 05 05 05 05 05   (all floor)
//   Row 5: 05 05 04 05 05   (floor, floor, floor+border, floor, floor — door row)
static const MapCell s_cells[] = {
    // row 0
    W(0x65), W(0x66), W(0x67), W(0x68), W(0x68),
    // row 1
    W(0x6B), W(0x6B), P(0x05), W(0x69), W(0x6A),
    // row 2 — 0x6D/0x6E are passable in the original (tile row-1 = 0x11 in Gym_Coll)
    P(0x05), P(0x05), P(0x05), P(0x6D), P(0x6E),
    // row 3 — pokeball counters (NPC sprites sit here; adjacent floor used to interact)
    W(0x68), W(0x68), P(0x05), W(0x68), W(0x68),
    // row 4 — open floor
    P(0x05), P(0x05), P(0x05), P(0x05), P(0x05),
    // row 5 — door row
    P(0x05), P(0x05), P(0x04), P(0x05), P(0x05),
};
#undef P
#undef W

static const MapLayout s_layout = {
    .width   = 5,
    .height  = 6,
    .tileset = &g_tileset_gym,
    .cells   = s_cells,
};

// Pokered warps (0-indexed in our system):
//   [0]: (4,11) → LAST_MAP dest_warp=2 → Pallet Town warp[2] = (12,11)
//   [1]: (5,11) → LAST_MAP dest_warp=2 → same (pokered: LAST_MAP,3 → 0-indexed=2)
// Entering from Pallet Town warp (OAKS_LAB,2 → 0-indexed=1) → spawn at warp[1]=(5,11)
static const WarpEvent s_warps[] = {
    { .x = 4, .y = 11, .dest_map = WARP_LAST_MAP, .dest_warp = 2 },
    { .x = 5, .y = 11, .dest_map = WARP_LAST_MAP, .dest_warp = 2 },
};

// NPCs: Rival (4,3), Oak (5,2), Pokéballs on the right counter at y=7.
// Pokéballs at y=7 (bottom subtile of the counter row, block row 3).
// With the -16px NPC render offset they appear at visual y=6 = top of the counter.
// Player at y=8 (floor) faces UP → interaction check reaches y=7.
// Sprite tiles: using 0=placeholder (16×16 white sprite) until art is in.
static const NpcDef s_npcs[] = {
    // Rival — stands at (4,3), facing south
    { .x = 4, .y = 3, .sprite_tile = GFX_BLUE_TILE_BASE, .facing = DIR_DOWN,  .flags = 0, .script_id = 1 },
    // Oak1 — final position (5,2), hidden until Oak2 walk-in finishes
    { .x = 5, .y = 2, .sprite_tile = GFX_OAK_TILE_BASE, .facing = DIR_DOWN,  .flags = NPCF_HIDDEN, .script_id = 2 },
    // Charmander Pokeball — right counter, left side
    { .x = 6, .y = 3, .sprite_tile = GFX_POKEBALL_TILE_BASE, .facing = DIR_DOWN,  .flags = 0, .script_id = 10 },
    // Bulbasaur Pokeball — right counter, center
    { .x = 7, .y = 3, .sprite_tile = GFX_POKEBALL_TILE_BASE, .facing = DIR_DOWN,  .flags = 0, .script_id = 12 },
    // Squirtle Pokeball — right counter, right side
    { .x = 8, .y = 3, .sprite_tile = GFX_POKEBALL_TILE_BASE, .facing = DIR_DOWN,  .flags = 0, .script_id = 11 },
    // Oak2 — temporary walk-in sprite. It only appears during the entrance
    // cutscene, then is hidden and replaced by Oak1 at (5,2).
    { .x = 5, .y = 10, .sprite_tile = GFX_OAK_TILE_BASE, .facing = DIR_UP,   .flags = NPCF_HIDDEN, .script_id = 0 },
};

extern void script_oaks_lab(void);

const MapHeader g_map_oaks_lab = {
    .map_id     = MAP_OAKS_LAB,
    .name       = "Oak's Lab",
    .layout     = &s_layout,
    .warps      = s_warps,
    .warp_count = 2,
    .npcs       = s_npcs,
    .npc_count  = 6,
    .script     = script_oaks_lab,
    .music_id   = 0,
};
