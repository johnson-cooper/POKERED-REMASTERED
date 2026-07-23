#include "world.h"
#include "gba.h"
#include "map_ids.h"
#include "metatile_ids.h"

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
        0x00, 0x10, 0x1B, 0x20, 0x21, 0x23, 0x2C, 0x2D, 0x2E,
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

    // Overworld: tile-based passability
    u16 mtid = MAPCELL_METATILE(cell);

    if (g_world.map->map_id == MAP_PALLET_TOWN &&
        block_x == 7 && block_y == 5)
        return FALSE;

    if (!layout->tileset || mtid >= layout->tileset->metatile_count)
        return FALSE;

    if (mtid == MT_ROUTE_BL || mtid == MT_ROUTE_BR)
        return FALSE;

    const Metatile *mt = &layout->tileset->metatiles[mtid];

    if (mtid == MT_GRASS_STEP)
        return TRUE;

    u32 local_col = (u32)((x & 1) * 2);
    u32 local_row = (u32)((y & 1) * 2 + 1);
    u32 left  = local_row * 4 + local_col;
    u32 right  = left + 1;

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

static const MapHeader * const s_map_table[MAP_COUNT] = {
    [MAP_PALLET_TOWN]        = &g_map_pallet_town,
    [MAP_ROUTE_1]            = &g_map_route_1,
    [MAP_PLAYERS_HOUSE_1F]   = &g_map_reds_house_1f,
    [MAP_RIVALS_HOUSE]       = &g_map_rivals_house,
    [MAP_PLAYERS_HOUSE_2F]   = &g_map_reds_house_2f,
    [MAP_OAKS_LAB]           = &g_map_oaks_lab,
};

const MapHeader *map_get_by_id(u8 map_id) {
    if (map_id >= MAP_COUNT) return NULL;
    return s_map_table[map_id];
}
