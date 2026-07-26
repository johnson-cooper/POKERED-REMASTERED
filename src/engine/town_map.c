#include "town_map.h"
#include "text.h"
#include "input.h"
#include "audio.h"
#include "world.h"
#include "map_ids.h"
#include "gba.h"

#define MAP_PAL 14

typedef struct {
    u8 map_id;
    u8 x;
    u8 y;
    const char *name;
} TownMapEntry;

static const TownMapEntry s_locations[] = {
    { MAP_PALLET_TOWN,    5,  15, "PALLET TOWN" },
    { MAP_VIRIDIAN_CITY,  5,  11, "VIRIDIAN CITY" },
    { MAP_PEWTER_CITY,    5,   5, "PEWTER CITY" },
    { MAP_CERULEAN_CITY, 14,   3, "CERULEAN CITY" },
    { MAP_VERMILION_CITY,14,  11, "VERMILION CITY" },
    { MAP_LAVENDER_TOWN, 21,   7, "LAVENDER TOWN" },
    { MAP_CELADON_CITY,  10,   7, "CELADON CITY" },
    { MAP_SAFFRON_CITY,  14,   7, "SAFFRON CITY" },
    { MAP_FUCHSIA_CITY,  14,  15, "FUCHSIA CITY" },
    { MAP_CINNABAR_ISLAND, 5, 17, "CINNABAR ISLAND" },
};

#define NUM_LOCATIONS ARRAY_COUNT(s_locations)

static const TownMapEntry s_routes[] = {
    { MAP_ROUTE_1, 5, 13, "ROUTE 1" },
    { MAP_ROUTE_2, 5,  8, "ROUTE 2" },
};

#define NUM_ROUTES ARRAY_COUNT(s_routes)

static bool8 s_open;
static u16 s_saved_dispcnt;
static u8 s_cursor;
static u8 s_blink_timer;

static u8 find_player_location(void) {
    if (!g_world.map) return 0;
    u8 mid = g_world.map->map_id;
    for (u8 i = 0; i < NUM_LOCATIONS; i++)
        if (s_locations[i].map_id == mid) return i;
    for (u8 i = 0; i < NUM_ROUTES; i++)
        if (s_routes[i].map_id == mid) return (u8)(NUM_LOCATIONS + i);
    // Indoor maps: resolve to parent town by map_id ranges.
    if (mid >= MAP_OAKS_LAB && mid <= MAP_RIVALS_HOUSE)
        return 0; // Pallet Town
    if (mid >= MAP_VIRIDIAN_MART && mid <= MAP_VIRIDIAN_GYM)
        return 1; // Viridian City
    return 0;
}

static const TownMapEntry *entry_for_index(u8 idx) {
    if (idx < NUM_LOCATIONS) return &s_locations[idx];
    if (idx < NUM_LOCATIONS + NUM_ROUTES) return &s_routes[idx - NUM_LOCATIONS];
    return &s_locations[0];
}

static void draw_map(void) {
    text_clear();

    // Draw border
    text_draw_tile(0, 0, BOX_TL);
    text_draw_tile(29, 0, BOX_TR);
    text_draw_tile(0, 19, BOX_BL);
    text_draw_tile(29, 19, BOX_BR);
    for (u8 c = 1; c < 29; c++) {
        text_draw_tile(c, 0, BOX_TE);
        text_draw_tile(c, 19, BOX_BE);
    }
    for (u8 r = 1; r < 19; r++) {
        text_draw_tile(0, r, BOX_LE);
        text_draw_tile(29, r, BOX_RE);
    }

    // Fill interior
    for (u8 r = 1; r < 19; r++)
        for (u8 c = 1; c < 29; c++)
            text_draw_tile(c, r, BOX_FILL);

    // Draw town dots
    for (u8 i = 0; i < NUM_LOCATIONS; i++)
        text_draw_char(s_locations[i].x, s_locations[i].y, 'o');

    // Draw route connections as dots
    for (u8 i = 0; i < NUM_ROUTES; i++)
        text_draw_char(s_routes[i].x, s_routes[i].y, '.');

    // Draw location name at bottom
    const TownMapEntry *loc = entry_for_index(s_cursor);
    for (u8 c = 1; c < 29; c++)
        text_draw_tile(c, 18, BOX_FILL);
    text_draw_str(2, 18, loc->name);
}

static void draw_cursor(void) {
    const TownMapEntry *loc = entry_for_index(s_cursor);
    if ((s_blink_timer / 16) & 1)
        text_draw_char(loc->x, loc->y, '#');
    else
        text_draw_char(loc->x, loc->y, 'o');
}

void town_map_open(void) {
    s_open = TRUE;
    s_cursor = find_player_location();
    s_blink_timer = 0;
    s_saved_dispcnt = REG_DISPCNT;
    REG_DISPCNT = (u16)((s_saved_dispcnt & (u16)~(DCNT_BG1 | DCNT_BG2 | DCNT_OBJ)) |
                        DCNT_BG0);
    draw_map();
}

bool8 town_map_update(void) {
    if (!s_open) return TRUE;

    s_blink_timer++;
    draw_cursor();

    u8 total = (u8)(NUM_LOCATIONS + NUM_ROUTES);
    if (input_pressed(KEY_UP)) {
        s_cursor = s_cursor == 0 ? (u8)(total - 1) : (u8)(s_cursor - 1);
        draw_map();
        audio_sfx_play(AUDIO_SFX_SELECT);
    }
    if (input_pressed(KEY_DOWN)) {
        s_cursor = s_cursor >= (u8)(total - 1) ? 0 : (u8)(s_cursor + 1);
        draw_map();
        audio_sfx_play(AUDIO_SFX_SELECT);
    }
    if (input_pressed(KEY_B) || input_pressed(KEY_A)) {
        return TRUE;
    }
    return FALSE;
}

void town_map_close(void) {
    if (!s_open) return;
    s_open = FALSE;
    text_init();
    tilemap_rebuild();
    tilemap_update_scroll();
    REG_DISPCNT = s_saved_dispcnt;
}
