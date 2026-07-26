#include "party_menu.h"
#include "audio.h"
#include "gba.h"
#include "gfx_party_icons.h"
#include "gfx_pokedex.h"
#include "experience.h"
#include "input.h"
#include "party.h"
#include "text.h"
#include "world.h"

#define PARTY_ICON_TILE 148
// Each of the 6 party slots gets 4 tiles (2×2 icon); sprite follows.
#define PARTY_SPRITE_TILE (PARTY_ICON_TILE + PARTY_SIZE * 4)
#define PARTY_SPRITE_SIZE 5
#define PARTY_SPRITE_PAL 11
#define PARTY_ICON_PAL 10
#define PARTY_ICON_FRAME_WORDS 32

typedef enum {
    PARTY_MENU_LIST = 0,
    PARTY_MENU_STATUS,
} PartyMenuMode;

static bool8 s_open;
static PartyMenuMode s_mode;
static u8 s_cursor;
static u16 s_saved_dispcnt;
static u16 s_saved_backdrop;
static u8 s_icon_frame;
static u8 s_icon_timer;

static void prepare_party_palettes(void) {
    vu16 *ui = (vu16 *)MEM_PAL + TEXT_PAL * 16;
    vu16 *bulba  = (vu16 *)MEM_PAL + 11 * 16;
    vu16 *charm  = (vu16 *)MEM_PAL + 12 * 16;
    vu16 *squirt = (vu16 *)MEM_PAL + 13 * 16;
    vu16 *pidgey = (vu16 *)MEM_PAL + 8 * 16;
    vu16 *rattat = (vu16 *)MEM_PAL + 9 * 16;

    for (u8 i = 0; i < 16; i++) {
        ui[i] = bulba[i] = charm[i] = squirt[i] = 0x7FFF;
        pidgey[i] = rattat[i] = 0x7FFF;
    }
    ui[1] = RGB15(2, 1, 2);
    ui[2] = RGB15(31, 31, 31);
    ui[3] = RGB15(12, 12, 12);
    ui[4] = RGB15(21, 21, 21);
    vu16 *icon = (vu16 *)MEM_PAL + PARTY_ICON_PAL * 16;
    for (u8 i = 0; i < 16; i++) icon[i] = ui[2];
    icon[1] = RGB15(21, 21, 21);
    icon[2] = RGB15(10, 10, 10);
    icon[3] = RGB15(2, 1, 2);
    icon[4] = RGB15(10, 10, 10);
    vu16 *hp = (vu16 *)MEM_PAL + 3 * 16;
    hp[1] = RGB15(4, 20, 7);
    hp[2] = ui[2];

    bulba[1] = charm[1] = squirt[1] = RGB15(2, 1, 2);
    pidgey[1] = rattat[1] = RGB15(2, 1, 2);
    bulba[2] = charm[2] = squirt[2] = ui[2];
    pidgey[2] = rattat[2] = ui[2];
    bulba[3]  = RGB15(5, 15, 4);
    bulba[4]  = RGB15(18, 28, 12);
    charm[3]  = RGB15(20, 5, 2);
    charm[4]  = RGB15(31, 16, 4);
    squirt[3] = RGB15(3, 9, 22);
    squirt[4] = RGB15(13, 23, 31);
    pidgey[3] = RGB15(18, 12, 8);
    pidgey[4] = RGB15(29, 24, 16);
    rattat[3] = RGB15(18, 9, 18);
    rattat[4] = RGB15(29, 18, 27);
}

static const u32 *party_sprite_tiles(PokemonId species) {
    switch (species) {
    case MON_CHARMANDER: return g_pokedex_charmander_tiles;
    case MON_SQUIRTLE:   return g_pokedex_squirtle_tiles;
    case MON_PIDGEY:     return g_pokedex_pidgey_tiles;
    case MON_RATTATA:    return g_pokedex_rattata_tiles;
    case MON_BULBASAUR:
    default:             return g_pokedex_bulbasaur_tiles;
    }
}

static const u32 *party_icon_tiles(PokemonId species) {
    switch (species) {
    case MON_BULBASAUR: return g_party_icon_grass_tiles;
    case MON_CHARMANDER: return g_party_icon_monster_tiles;
    case MON_SQUIRTLE: return g_party_icon_water_tiles;
    case MON_PIDGEY: return g_party_icon_bird_tiles;
    case MON_RATTATA: return g_party_icon_quadruped_tiles;
    default: return g_party_icon_monster_tiles;
    }
}

static u8 party_sprite_palette(PokemonId species) {
    switch (species) {
    case MON_CHARMANDER: return 12;
    case MON_SQUIRTLE:   return 13;
    case MON_PIDGEY:     return 8;
    case MON_RATTATA:    return 9;
    case MON_BULBASAUR:
    default:             return PARTY_SPRITE_PAL;
    }
}

static const char *party_species_name(PokemonId species) {
    switch (species) {
    case MON_BULBASAUR: return "BULBASAUR";
    case MON_CHARMANDER: return "CHARMANDER";
    case MON_SQUIRTLE: return "SQUIRTLE";
    case MON_PIDGEY: return "PIDGEY";
    case MON_RATTATA: return "RATTATA";
    default: return "POKEMON";
    }
}

static const char *party_status_name(u8 status) {
    switch (status) {
    case 1: return "SLP";
    case 2: return "PSN";
    case 3: return "BRN";
    case 4: return "FRZ";
    case 5: return "PAR";
    default: return "OK";
    }
}

static const char *party_type1(PokemonId species) {
    switch (species) {
    case MON_CHARMANDER: return "FIRE";
    case MON_SQUIRTLE: return "WATER";
    default: return "GRASS";
    }
}

static const char *party_type2(PokemonId species) {
    switch (species) {
    case MON_BULBASAUR: return "POISON";
    default: return "";
    }
}

static char *party_number(u32 value) {
    static char buffer[11];
    u8 i = 0;
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return buffer;
    }
    while (value && i < 10) {
        buffer[i++] = (char)('0' + value % 10);
        value /= 10;
    }
    for (u8 left = 0; left < i / 2; left++) {
        char c = buffer[left];
        buffer[left] = buffer[i - left - 1];
        buffer[i - left - 1] = c;
    }
    buffer[i] = '\0';
    return buffer;
}

static u32 party_exp_to_next_level(const PartyPokemon *mon) {
    if (mon->level >= 100) return 0;
    u32 required = pokemon_exp_for_level(mon->species, (u8)(mon->level + 1));
    return mon->experience < required ? required - mon->experience : 0;
}

const char *party_mon_display_name(const PartyPokemon *mon) {
    if (mon->nickname[0]) return mon->nickname;
    return party_species_name(mon->species);
}

static void party_fill(void) {
    text_fill_opaque();
}

static void party_box(u8 left, u8 top, u8 right, u8 bottom) {
    for (u8 y = top; y <= bottom; y++) {
        for (u8 x = left; x <= right; x++) {
            u8 tile = BOX_FILL;
            if (x == left && y == top) tile = BOX_TL;
            else if (x == right && y == top) tile = BOX_TR;
            else if (x == left && y == bottom) tile = BOX_BL;
            else if (x == right && y == bottom) tile = BOX_BR;
            else if (y == top) tile = BOX_TE;
            else if (y == bottom) tile = BOX_BE;
            else if (x == left) tile = BOX_LE;
            else if (x == right) tile = BOX_RE;
            text_draw_tile_pal(x, y, tile, TEXT_PAL);
        }
    }
}

static void load_party_picture(const PartyPokemon *mon, u8 col, u8 row) {
    const u32 *tiles = party_sprite_tiles(mon->species);
    u8 palette = party_sprite_palette(mon->species);
    vu32 *vram = (vu32 *)(MEM_VRAM + 0x4000);
    for (u32 i = 0; i < 200; i++)
        vram[(PARTY_SPRITE_TILE * 8) + i] = tiles[i];
    for (u8 y = 0; y < PARTY_SPRITE_SIZE; y++)
        for (u8 x = 0; x < PARTY_SPRITE_SIZE; x++)
            text_draw_tile_pal((u8)(col + x), (u8)(row + y),
                               (u8)(PARTY_SPRITE_TILE + y * PARTY_SPRITE_SIZE + x),
                               palette);
}

static void load_party_icon(const PartyPokemon *mon, u8 slot, u8 col, u8 row) {
    const u32 *tiles = party_icon_tiles(mon->species);
    vu32 *vram = (vu32 *)(MEM_VRAM + 0x4000);
    tiles += s_icon_frame * PARTY_ICON_FRAME_WORDS;
    // Each party slot owns 4 consecutive tile slots (2×2) so icons don't
    // overwrite each other in VRAM. Tile layout: left-top, right-top (mirrored),
    // left-bottom, right-bottom (mirrored) — matching pokered's OAM approach.
    u16 base = (u16)(PARTY_ICON_TILE + slot * 4);
    for (u8 row_tile = 0; row_tile < 2; row_tile++) {
        for (u8 py = 0; py < 8; py++) {
            u32 source = tiles[row_tile * 16 + py];
            u32 flipped = 0;
            for (u8 pixel = 0; pixel < 8; pixel++)
                flipped |= ((source >> (pixel * 4)) & 0xF) << ((7 - pixel) * 4);
            vram[base * 8 + row_tile * 16 + py]     = source;
            vram[base * 8 + row_tile * 16 + 8 + py] = flipped;
        }
    }
    for (u8 y = 0; y < 2; y++)
        for (u8 x = 0; x < 2; x++)
            text_draw_tile_pal((u8)(col + x), (u8)(row + y),
                               (u8)(base + y * 2 + x),
                               PARTY_ICON_PAL);
}

static void draw_hp(const PartyPokemon *mon, u8 col, u8 row) {
    u8 filled = 0;
    if (mon->max_hp) filled = (u8)((mon->current_hp * 6U) / mon->max_hp);
    text_draw_str(col, row, "HP:");
    for (u8 i = 0; i < 6; i++)
        text_draw_tile_pal((u8)(col + 3 + i), row,
                           i < filled ? HP_BAR_FILL_TILE : HP_BAR_EMPTY_TILE,
                           3);
    text_draw_str((u8)(col + 10), row, party_number(mon->current_hp));
    text_draw_char((u8)(col + 13), row, '/');
    text_draw_str((u8)(col + 14), row, party_number(mon->max_hp));
}

static void draw_list(void) {
    party_fill();
    party_box(0, 13, 29, 19);
    text_draw_str(2, 15, "CHOOSE A POKEMON.");
    text_draw_str(2, 18, "A:STATUS   B:BACK");

    if (!g_party.count) {
        text_draw_str(2, 1, "NO POKEMON");
        return;
    }

    for (u8 i = 0; i < g_party.count && i < PARTY_SIZE; i++) {
        const PartyPokemon *mon = &g_party.mons[i];
        u8 col = (i & 1) ? 16 : 1;
        u8 row = (u8)(1 + (i / 2) * 4);
        if (i == s_cursor) text_draw_char((u8)(col - 1), (u8)(row + 1), '>');
        load_party_icon(mon, i, col, row);
        text_draw_str_n((u8)(col + 3), row, party_mon_display_name(mon), 11);
        text_draw_str((u8)(col + 3), (u8)(row + 1), ":LV");
        text_draw_str((u8)(col + 6), (u8)(row + 1), party_number(mon->level));
        text_draw_str((u8)(col + 3), (u8)(row + 2), "HP");
        text_draw_str((u8)(col + 6), (u8)(row + 2), party_number(mon->current_hp));
        text_draw_char((u8)(col + 9), (u8)(row + 2), '/');
        text_draw_str((u8)(col + 10), (u8)(row + 2), party_number(mon->max_hp));
    }
}

static void draw_status(void) {
    const PartyPokemon *mon = &g_party.mons[s_cursor];
    party_fill();
    party_box(10, 0, 29, 5);
    text_draw_str(6, 0, party_mon_display_name(mon));
    text_draw_str(21, 1, ":LV");
    text_draw_str(25, 1, party_number(mon->level));
    text_draw_str(14, 3, "STATUS");
    text_draw_str(24, 3, party_status_name(mon->status));
    draw_hp(mon, 14, 4);

    load_party_picture(mon, 1, 1);
    text_draw_str(1, 7, "NO.");
    text_draw_str(5, 7, party_number((u16)mon->species));

    party_box(0, 9, 11, 19);
    text_draw_str(1, 10, "ATTACK");
    text_draw_str(8, 10, party_number(mon->attack));
    text_draw_str(1, 12, "DEFENSE");
    text_draw_str(8, 12, party_number(mon->defense));
    text_draw_str(1, 14, "SPEED");
    text_draw_str(8, 14, party_number(mon->speed));
    text_draw_str(1, 16, "SPECIAL");
    text_draw_str(8, 16, party_number(mon->special));

    party_box(13, 7, 29, 19);
    text_draw_str(14, 8, "TYPE1/");
    text_draw_str(22, 8, party_type1(mon->species));
    text_draw_str(14, 10, "TYPE2/");
    text_draw_str(22, 10, party_type2(mon->species));
    text_draw_str(14, 12, "STATUS/");
    text_draw_str(22, 12, party_status_name(mon->status));
    text_draw_str(14, 14, "EXP");
    text_draw_str(22, 14, party_number(mon->experience));
    text_draw_str(14, 16, "NEXT");
    text_draw_str(22, 16, party_number(party_exp_to_next_level(mon)));
    if (s_cursor == 0)
        text_draw_str(14, 18, "B:BACK");
    else
        text_draw_str(14, 18, "A:LEAD  B:BACK");
}

void party_menu_open(void) {
    s_open = TRUE;
    s_mode = PARTY_MENU_LIST;
    s_cursor = 0;
    s_icon_frame = 0;
    s_icon_timer = 0;
    s_saved_dispcnt = REG_DISPCNT;
    s_saved_backdrop = PAL_BG[0];
    REG_DISPCNT = (u16)((s_saved_dispcnt & (u16)~(DCNT_BG1 | DCNT_BG2 | DCNT_OBJ)) |
                        DCNT_BG0);
    PAL_BG[0] = RGB15(31, 31, 31);
    prepare_party_palettes();
    if (g_party.count) draw_list();
    else {
        party_fill();
        party_box(0, 0, 29, 19);
        text_draw_str(10, 1, "POKEMON");
        text_draw_str(7, 9, "NO POKEMON");
        text_draw_str(2, 18, "B:BACK");
    }
}

bool8 party_menu_update(void) {
    if (!s_open) return TRUE;
    if (s_mode == PARTY_MENU_LIST && g_party.count) {
        if (++s_icon_timer >= 8) {
            s_icon_timer = 0;
            s_icon_frame ^= 1;
            draw_list();
        }
    }
    if (s_mode == PARTY_MENU_STATUS && s_cursor != 0 && input_pressed(KEY_A)) {
        audio_sfx_play(AUDIO_SFX_CONFIRM);
        party_swap_slots(0, s_cursor);
        s_cursor = 0;
        s_mode = PARTY_MENU_LIST;
        draw_list();
        return FALSE;
    }
    if (input_pressed(KEY_B)) {
        if (s_mode == PARTY_MENU_STATUS) {
            s_mode = PARTY_MENU_LIST;
            draw_list();
            return FALSE;
        }
        party_menu_close();
        return TRUE;
    }
    if (s_mode == PARTY_MENU_LIST && g_party.count) {
        bool8 moved = FALSE;
        if (input_pressed(KEY_UP)) {
            s_cursor = s_cursor ? (u8)(s_cursor - 1) : (u8)(g_party.count - 1);
            moved = TRUE;
        } else if (input_pressed(KEY_DOWN)) {
            s_cursor = (u8)((s_cursor + 1) % g_party.count);
            moved = TRUE;
        }
        if (moved) {
            audio_sfx_play(AUDIO_SFX_SELECT);
            draw_list();
        }
        if (input_pressed(KEY_A)) {
            audio_sfx_play(AUDIO_SFX_CONFIRM);
            s_mode = PARTY_MENU_STATUS;
            draw_status();
        }
    }
    return FALSE;
}

void party_menu_close(void) {
    s_open = FALSE;
    text_init();
    tilemap_rebuild();
    PAL_BG[0] = s_saved_backdrop;
    REG_DISPCNT = s_saved_dispcnt;
}
