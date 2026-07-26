#include "world.h"
#include "gba.h"
#include "map_ids.h"
#include "metatile_ids.h"
#include "gfx_overworld.h"

WorldContext g_world;

void map_load(const MapHeader *map) {
    g_world.map = map;
}

MapCell map_get_cell(s32 x, s32 y) {
    const MapLayout *layout = g_world.map->layout;
    if (x < 0 || y < 0 || x >= layout->width || y >= layout->height)
        return MAPCELL_IMPASSABLE;
    return layout->cells[y * layout->width + x];
}

bool8 map_is_passable(s32 x, s32 y) {
    MapCell cell = map_get_cell(x, y);
    return MAPCELL_COLLISION(cell) == 0;
}

static bool8 overworld_tile_is_passable(u16 tile_id) {
    static const u8 s_passable_tiles[] = {
        0x00, 0x10, 0x11, 0x1B, 0x20, 0x21, 0x23, 0x2C, 0x2D, 0x2E,
        0x30, 0x31, 0x33, 0x39, 0x3C, 0x3E, 0x52, 0x54, 0x58, 0x5B,
    };
    for (u32 i = 0; i < ARRAY_COUNT(s_passable_tiles); i++) {
        if (tile_id == s_passable_tiles[i])
            return TRUE;
    }
    return FALSE;
}

static bool8 overworld_tile_is_flower(u16 tile_id, u16 metatile_id) {
    return (metatile_id == MT_GRASS_STEP && tile_id == 0x4B) ||
           (metatile_id == MT_ROAD_H && tile_id == 0x03);
}

static bool8 overworld_tile_is_fence_overlay(u16 tile_id) {
    return tile_id == (0x0E + OVERWORLD_OVERLAY_EDGE_MASK_BASE) ||
           tile_id == (0x55 + OVERWORLD_OVERLAY_EDGE_MASK_BASE) ||
           tile_id == (0x0E + OVERWORLD_OVERLAY_SOLID_BASE) ||
           tile_id == (0x55 + OVERWORLD_OVERLAY_SOLID_BASE);
}

static bool8 indoor_tile_is_passable(const Tileset *ts, u16 tile) {
    for (u8 i = 0; i < ts->collision_tile_count; i++) {
        if (tile == ts->collision_tiles[i])
            return TRUE;
    }
    return FALSE;
}

bool8 map_is_subtile_passable_from(s32 x, s32 y, Direction dir) {
    (void)dir;
    return map_is_subtile_passable(x, y);
}

u16 map_get_subtile_tile_id(s32 x, s32 y) {
    const MapLayout *layout = g_world.map->layout;
    if (x < 0 || y < 0)
        return 0xFFFF;

    s32 block_x = x / 2;
    s32 block_y = y / 2;
    if (block_x >= layout->width || block_y >= layout->height)
        return 0xFFFF;

    MapCell cell = map_get_cell(block_x, block_y);
    u16 mtid = MAPCELL_METATILE(cell);

    if (!layout->tileset || mtid >= layout->tileset->metatile_count)
        return 0xFFFF;

    const Metatile *mt = &layout->tileset->metatiles[mtid];
    u32 col = (u32)(x & 1) * 2;
    u32 row = (u32)(y & 1) * 2 + 1;
    return mt->bottom[row * 4 + col];
}

bool8 map_is_subtile_passable(s32 x, s32 y) {
    const MapLayout *layout = g_world.map->layout;

    if (x < 0 || y < 0)
        return FALSE;

    s32 block_x = x / 2;
    s32 block_y = y / 2;
    if (block_x >= layout->width || block_y >= layout->height)
        return FALSE;

    MapCell cell = map_get_cell(block_x, block_y);

    // Indoor maps: always check tile content, ignoring MapCell collision flag.
    // For any block, check all 4 tiles in the subtile region; allow if ANY are passable.
    // This lets players access subtiles with mixed content (e.g., floor on left, deco on right).
    if (layout->tileset && layout->tileset->use_cell_collision) {
        const Tileset *ts = layout->tileset;
        if (!ts->collision_tiles || ts->collision_tile_count == 0)
            return FALSE;
        if (!ts->collision_tiles || ts->collision_tile_count == 0) {
            // Fallback: use original 4-tile all-floor check if no collision list defined
            if (MAPCELL_COLLISION(cell) == 0) return TRUE;
            u16 mtid = MAPCELL_METATILE(cell);
            if (mtid >= ts->metatile_count) return FALSE;
            const Metatile *mt = &ts->metatiles[mtid];
            u32 col = (u32)(x & 1) * 2;
            u32 row = (u32)(y & 1) * 2;
            u16 t00 = mt->bottom[row * 4 + col];
            u16 t01 = mt->bottom[row * 4 + col + 1];
            u16 t10 = mt->bottom[(row+1) * 4 + col];
            u16 t11 = mt->bottom[(row+1) * 4 + col + 1];
            bool8 all_floor = (t00 == 0x01 && t01 == 0x01 &&
                               t10 == 0x01 && t11 == 0x01) ||
                              (t00 == 0x11 && t01 == 0x11 &&
                               t10 == 0x11 && t11 == 0x11);
            return all_floor;
        }
        u16 mtid = MAPCELL_METATILE(cell);
        if (mtid >= ts->metatile_count) return FALSE;
        const Metatile *mt = &ts->metatiles[mtid];
        u32 col = (u32)(x & 1) * 2;
        u32 row = (u32)(y & 1) * 2 + 1;
        // Check all 4 tiles in the 2×2 subtile region (rows `row` and `row+1`).
        // Passable if any tile is in the collision_tiles list.
        return indoor_tile_is_passable(ts, mt->bottom[row * 4 + col]);
    }

    u16 mtid = MAPCELL_METATILE(cell);

    // Viridian buildings use ordinary P() map cells, so their collision
    // must be supplied explicitly rather than inferred from MAPCELL_W().
    if (g_world.map->map_id == MAP_VIRIDIAN_CITY &&
        (mtid == 0x02 || mtid == 0x03 || mtid == 0x0C ||
         mtid == 0x0D || mtid == 0x0E || mtid == 0x11 ||
         mtid == 0x12 || mtid == 0x20 || mtid == 0x21 ||
         mtid == 0x72 || mtid == 0x73 ||
         mtid == 0x7C || mtid == 0x7D || mtid == 0x7E ||
         mtid == 0x7F))
        return FALSE;

    // 0x77 renders the Viridian fence in the lower half of its 2x2 block;
    // keep the grass strip above it walkable so the fence occupies one
    // movement row rather than two.
    if (g_world.map->map_id == MAP_VIRIDIAN_CITY && mtid == 0x77 &&
        (y & 1) != 0)
        return FALSE;

    // Apply explicit collision only to true building/fence objects. Borders,
    // trees, and mixed ground blocks retain the original tile-based rules;
    // their W() map flag does not mean every subtile is solid.
    if (MAPCELL_COLLISION(cell) != 0) {
        if ((mtid == MT_BLDG_TOP_L || mtid == MT_BLDG_TOP_R ||
             mtid == MT_SIGN ||
             mtid == MT_ROAD_CORNER) &&
            (y & 1) == 0)
            return TRUE;
        if (mtid == MT_BLDG_TOP_L || mtid == MT_BLDG_TOP_R ||
            mtid == MT_SIGN ||
            mtid == MT_ROAD_CORNER ||
            mtid == MT_BLDG_LEFT || mtid == MT_BLDG_MID ||
            mtid == MT_BLDG_RIGHT || mtid == MT_BLDG_DOOR ||
            mtid == MT_BLDG_WALL_L || mtid == MT_BLDG_WALL_R)
            return FALSE;
    }

    if (g_world.map->map_id == MAP_PALLET_TOWN &&
        block_x == 7 && block_y == 5)
        return FALSE;

    if (!layout->tileset || mtid >= layout->tileset->metatile_count)
        return FALSE;

    if (mtid == MT_ROUTE_BL || mtid == MT_ROUTE_BR)
        return FALSE;

    const Metatile *mt = &layout->tileset->metatiles[mtid];

    u32 local_col = (u32)((x & 1) * 2);
    u32 local_row = (u32)((y & 1) * 2 + 1);
    u32 left  = local_row * 4 + local_col;
    u32 right  = left + 1;

    // Collide only on the subtile containing the visible fence overlay;
    // leave the surrounding grass in the same map cell walkable.
    if (g_world.map->map_id == MAP_VIRIDIAN_CITY &&
        (overworld_tile_is_fence_overlay(mt->top[left]) ||
         overworld_tile_is_fence_overlay(mt->top[right])))
        return FALSE;

    // Some Viridian fences are top-layer objects over grass. Only the fence
    // overlay variants are solid; other top-layer decorations remain normal.
    if (mtid == MT_GRASS_STEP)
        return TRUE;

    // Viridian perimeter border blocks are solid. On other maps these same
    // metatile IDs may be reused for walkable terrain (e.g. 0x0B = grass on Route 1).
    if (g_world.map->map_id == MAP_VIRIDIAN_CITY &&
        (mtid == MT_BORDER_L  || mtid == MT_BORDER_R  ||
         mtid == MT_BORDER_TL || mtid == MT_BORDER_TR ||
         mtid == MT_BORDER_BL))
        return FALSE;

    return (overworld_tile_is_passable(mt->bottom[left]) ||
            overworld_tile_is_flower(mt->bottom[left], mtid)) &&
           (overworld_tile_is_passable(mt->bottom[right]) ||
            overworld_tile_is_flower(mt->bottom[right], mtid));
}

// ── Map registry ──────────────────────────────────────────────────────────────
extern const MapHeader g_map_pallet_town;
extern const MapHeader g_map_route_1;
extern const MapHeader g_map_reds_house_2f;
extern const MapHeader g_map_reds_house_1f;
extern const MapHeader g_map_rivals_house;
extern const MapHeader g_map_oaks_lab;
extern const MapHeader g_map_viridian_city;
extern const MapHeader g_map_viridian_mart;
extern const MapHeader g_map_viridian_pokecenter;
extern const MapHeader g_map_viridian_school_house;
extern const MapHeader g_map_viridian_nickname_house;
extern const MapHeader g_map_viridian_gym;

static const MapHeader * const s_map_table[MAP_COUNT] = {
    [MAP_PALLET_TOWN]        = &g_map_pallet_town,
    [MAP_ROUTE_1]            = &g_map_route_1,
    [MAP_PLAYERS_HOUSE_1F]   = &g_map_reds_house_1f,
    [MAP_RIVALS_HOUSE]       = &g_map_rivals_house,
    [MAP_PLAYERS_HOUSE_2F]   = &g_map_reds_house_2f,
    [MAP_OAKS_LAB]           = &g_map_oaks_lab,
    [MAP_VIRIDIAN_CITY]      = &g_map_viridian_city,
    [MAP_VIRIDIAN_MART]      = &g_map_viridian_mart,
    [MAP_VIRIDIAN_POKECENTER] = &g_map_viridian_pokecenter,
    [MAP_VIRIDIAN_SCHOOL_HOUSE] = &g_map_viridian_school_house,
    [MAP_VIRIDIAN_NICKNAME_HOUSE] = &g_map_viridian_nickname_house,
    [MAP_VIRIDIAN_GYM]           = &g_map_viridian_gym,
};

const MapHeader *map_get_by_id(u8 map_id) {
    if (map_id >= MAP_COUNT) return NULL;
    return s_map_table[map_id];
}
