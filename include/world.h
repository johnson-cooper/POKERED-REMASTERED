#pragma once
#include "types.h"

// ── Map grid cell ─────────────────────────────────────────────────────────────
typedef u16 MapCell;

#define MAPCELL_METATILE(c)   ((c) & 0x03FF)
#define MAPCELL_COLLISION(c)  (((c) >> 10) & 0x3)
#define MAPCELL_ELEVATION(c)  (((c) >> 12) & 0xF)
#define MAPCELL_MAKE(m,col,el) (((el)<<12)|((col)<<10)|((m)&0x3FF))
#define MAPCELL_IMPASSABLE     0x0400

// Directional collision edges for one 16px movement cell.  A set bit means
// that entering the cell through that edge is blocked.
#define COLLISION_EDGE_NORTH  (1u << 0)
#define COLLISION_EDGE_SOUTH  (1u << 1)
#define COLLISION_EDGE_WEST   (1u << 2)
#define COLLISION_EDGE_EAST   (1u << 3)

// ── Map block ─────────────────────────────────────────────────────────────────
typedef struct {
    u16 bottom[16];
    u16 top[16];
    u8  palettes[16];
    u8  top_palettes[16];
} Metatile;

typedef struct {
    const u16 *colors;
    u8 count;
} PaletteProfile;

// Per-town roof colors replacing all four meaningful shades of the ROOF
// palette slot (palette 6).  The reference provides only the two lightest;
// we derive a matching dark-mid shade so the stripe pattern doesn't use the
// green OUTDOOR_ROOF base color.
typedef struct {
    u16 lightest;   // palette index 15 (GB shade 0)
    u16 light;      // palette index 10 (GB shade 1)
    u16 dark;       // palette index  5 (GB shade 2)
    u16 darkest;    // palette index  1 (GB shade 3)
} RoofPalette;

// ── Tileset ───────────────────────────────────────────────────────────────────
typedef struct {
    const u32     *tiles;
    const u32     *overlay_tiles;
    u32            tile_count;
    const u16     *palettes;
    u8             palette_count;
    const PaletteProfile *palette_profile;
    const u8      *tile_palette_map;
    const Metatile *metatiles;
    u16            metatile_count;
    // Optional pokered-style directional collision masks, indexed by
    // metatile ID. A NULL table means the map code derives the mask from the
    // overworld collision tile list.
    const u8      *collision_edge_masks;
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
    // Optional per-movement-cell masks for visual overlays (houses, fences,
    // trees). The valid table distinguishes an override from an ordinary
    // pokered tile-sampled cell.
    const u8       *collision_subtile_masks;
    const u8       *collision_subtile_mask_valid;
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

// Pokered map-header connections join outdoor maps at an edge instead of
// using an authored doorway warp. `offset` is applied to the source edge
// coordinate when calculating the destination spawn coordinate.
typedef struct {
    u8 direction;    // DIR_UP/DOWN/LEFT/RIGHT
    u8 dest_map;
    s8 offset;
} MapConnection;

// Background events are non-NPC interactions such as signs and wall-mounted
// text. Coordinates use the same 16px subtile units as WarpEvent and NpcDef.
typedef struct {
    u8  x, y;
    u16 script_id;
    const char *text;    // pokered TEXT_* payload for a simple sign/poster
} BackgroundEvent;

// ── NPC ───────────────────────────────────────────────────────────────────────
typedef struct {
    u8  x, y;           // subtile position (16px units)
    u16 sprite_tile;    // OBJ tile index (from NPC sprite sheet)
    u8  facing;         // Direction
    u8  flags;          // NPCF_* flags
    u16 script_id;
    u8  movement;       // NpcMovement
    u16 item_id;        // ITEM_* when NPCF_ITEM is set
    u16 item_flag;      // persistent GameFlag used to hide a collected item
    const char *text;    // simple TEXT_* payload; NULL means scripted behavior
    u8  trainer_id;     // TrainerId when NPCF_TRAINER is set
    u8  trainer_party;   // reference party index for future trainer variants
    u8  trainer_sight;   // pokered trainer engage distance in tiles
    u16 trainer_flag;    // persistent defeated flag; 0 means non-persistent
    const char *trainer_text; // dialogue shown immediately before battle
} NpcDef;

// Runtime copy of an NPC definition. Scripted scenes can move and hide these
// without mutating the read-only map data.
typedef struct {
    u8  x, y;
    u16 sprite_tile;
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
    u16 item_id;
    u16 item_flag;
    const char *text;
    u8  trainer_id;
    u8  trainer_party;
    u8  trainer_sight;
    u16 trainer_flag;
    const char *trainer_text;
} NpcState;

#define NPCF_HIDDEN     0x01  // not rendered or interactive
#define NPCF_NO_SPRITE  0x02  // tile provides the visual; NPC is still interactive
#define NPCF_ITEM       0x04  // overworld item ball; item_id/item_flag are valid
#define NPCF_TRAINER    0x08  // starts a data-driven trainer battle
#define NPCF_TRAINER_DEFEATED 0x10 // trainer remains visible but will not re-battle

typedef enum {
    NPC_MOVE_STAY = 0,
    NPC_MOVE_WALK_ANY,
    NPC_MOVE_UP_DOWN,
    NPC_MOVE_LEFT_RIGHT,
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
    const RoofPalette *roof_palette;
    const BackgroundEvent *bg_events;
    u8              bg_event_count;
    const MapConnection *connections;
    u8              connection_count;
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
    bool8     ledge_jumping;
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
void world_do_connection(u8 dest_map, Direction direction, s16 source_coordinate,
                         s8 offset);
void world_npc_start_step(u8 index, Direction dir);
bool8 world_npc_is_moving(u8 index);

// Map API
void     map_load(const MapHeader *map);
MapCell  map_get_cell(s32 x, s32 y);
bool8    map_is_passable(s32 x, s32 y);
bool8    map_is_subtile_passable(s32 x, s32 y);
bool8    map_is_subtile_passable_from(s32 x, s32 y, Direction dir);
u8       map_get_subtile_collision_edges(s32 x, s32 y);
u16      map_get_subtile_tile_id(s32 x, s32 y);

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
void tilemap_apply_roof_palette(const RoofPalette *roof);
void tilemap_load_player_sprite(void);
void tilemap_rebuild(void);
void tilemap_update_scroll(void);
