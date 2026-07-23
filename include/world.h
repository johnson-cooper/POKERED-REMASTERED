#pragma once
#include "types.h"

// ── Map grid cell ─────────────────────────────────────────────────────────────
typedef u16 MapCell;

#define MAPCELL_METATILE(c)   ((c) & 0x03FF)
#define MAPCELL_COLLISION(c)  (((c) >> 10) & 0x3)
#define MAPCELL_ELEVATION(c)  (((c) >> 12) & 0xF)
#define MAPCELL_MAKE(m,col,el) (((el)<<12)|((col)<<10)|((m)&0x3FF))
#define MAPCELL_IMPASSABLE     0x0400

// ── Map block ─────────────────────────────────────────────────────────────────
typedef struct {
    u16 bottom[16];
    u16 top[16];
    u8  palettes[16];
    u8  top_palettes[16];
} Metatile;

// ── Tileset ───────────────────────────────────────────────────────────────────
typedef struct {
    const u32     *tiles;
    u32            tile_count;
    const u16     *palettes;
    u8             palette_count;
    const Metatile *metatiles;
    u16            metatile_count;
    bool8          use_cell_collision; // TRUE = indoor: refine W cells per-subtile by tile content
    const u8      *collision_tiles;    // tile IDs that indicate passable ground (indoor only)
    u8             collision_tile_count;
} Tileset;

// ── Map layout ────────────────────────────────────────────────────────────────
typedef struct {
    s32             width;
    s32             height;
    const Tileset  *tileset;
    const MapCell  *cells;
} MapLayout;

// ── Warp ──────────────────────────────────────────────────────────────────────
// Coordinates are in 16px subtile units, matching pokered map-square coords.
// dest_map == 0xFF means "return to g_world.last_map" (LAST_MAP in pokered).
#define WARP_LAST_MAP 0xFF

typedef struct {
    u8 x, y;
    u8 dest_map;
    u8 dest_warp;   // 0-indexed into destination map's warp array
} WarpEvent;

// ── NPC ───────────────────────────────────────────────────────────────────────
typedef struct {
    u8  x, y;           // subtile position (16px units)
    u8  sprite_tile;    // OBJ tile index (from NPC sprite sheet)
    u8  facing;         // Direction
    u8  flags;          // NPCF_* flags
    u16 script_id;
    u8  movement;       // NpcMovement
} NpcDef;

// Runtime copy of an NPC definition. Scripted scenes can move and hide these
// without mutating the read-only map data.
typedef struct {
    u8  x, y;
    u8  sprite_tile;
    u8  facing;
    u8  flags;
    u16 script_id;
    s16 px, py;
    s8  step_dx, step_dy;
    u8  step_dir;
    u8  step_frame;
    bool8 walking;
    u8  walk_cycle;
    u8  movement;       // NpcMovement
    u16 move_timer;
} NpcState;

#define NPCF_HIDDEN  0x01  // not rendered or interactive

typedef enum {
    NPC_MOVE_STAY = 0,
    NPC_MOVE_WALK_ANY,
    NPC_MOVE_UP_DOWN,
} NpcMovement;

// ── Map script ────────────────────────────────────────────────────────────────
typedef void (*MapScriptFn)(void);

// ── Map header ────────────────────────────────────────────────────────────────
typedef struct {
    u8              map_id;
    const char     *name;
    const MapLayout *layout;
    const WarpEvent *warps;
    u8              warp_count;
    const NpcDef   *npcs;
    u8              npc_count;
    MapScriptFn     script;
    u16             music_id;
} MapHeader;

// ── Player ────────────────────────────────────────────────────────────────────
typedef enum {
    DIR_DOWN  = 0,
    DIR_UP    = 1,
    DIR_LEFT  = 2,
    DIR_RIGHT = 3,
} Direction;

typedef enum {
    MOVE_STATE_IDLE = 0,
    MOVE_STATE_TURNING,
    MOVE_STATE_WALKING,
    MOVE_STATE_FROZEN,   // scripted movement, input blocked
} MoveState;

typedef struct {
    s16       tile_x, tile_y;
    s16       px, py;
    Direction facing;
    MoveState move_state;
    u8        step_frame;
    s16       step_dx, step_dy;
    u8        walk_cycle;
} PlayerState;

// ── Camera ────────────────────────────────────────────────────────────────────
typedef struct { s32 x, y; } Camera;

// ── World context ─────────────────────────────────────────────────────────────
typedef struct {
    const MapHeader *map;
    const MapHeader *last_map;   // for WARP_LAST_MAP (LAST_MAP / $FF)
    PlayerState      player;
    Camera           camera;
    NpcState         npcs[16];
    u8               npc_count;
} WorldContext;

extern WorldContext g_world;

// ── World API ─────────────────────────────────────────────────────────────────
void world_init(const MapHeader *map, u8 start_x, u8 start_y);
void world_update(void);
void world_render(void);
void world_do_warp(const WarpEvent *w);
void world_npc_start_step(u8 index, Direction dir);
bool8 world_npc_is_moving(u8 index);

// Map API
void     map_load(const MapHeader *map);
MapCell  map_get_cell(s32 x, s32 y);
bool8    map_is_passable(s32 x, s32 y);
bool8    map_is_subtile_passable(s32 x, s32 y);
bool8    map_is_subtile_passable_from(s32 x, s32 y, Direction dir);

// Map registry
const MapHeader *map_get_by_id(u8 map_id);

// Player / camera
void player_update(void);
bool8 player_script_start_step(Direction dir);
bool8 player_script_start_step_forced(Direction dir);
bool8 player_script_is_moving(void);
void camera_update(void);

// Tilemap (GBA BG hardware)
void tilemap_init(void);
void tilemap_load_tileset(const Tileset *ts);
void tilemap_load_player_sprite(void);
void tilemap_rebuild(void);
void tilemap_update_scroll(void);
