#include "pokedex.h"
#include "dialog.h"
#include "gba.h"
#include "gfx_pokedex.h"
#include "input.h"
#include "text.h"
#include "world.h"
#include "audio.h"

#define POKEDEX_SPRITE_TILE 140
#define POKEDEX_PAL 14
#define POKEDEX_TILE_VRAM ((u32 *)(MEM_VRAM + 0x4000))

typedef struct {
    const char *name;
    const char *category;
    const char *height;
    const char *weight;
    const char *number;
    const char *description[3];
    const char *description_page2[3];
    const u32 *tiles;
    u8 sprite_palette;
} PokedexEntry;

static const PokedexEntry s_entries[] = {
    {
        "BULBASAUR", "SEED", "HT 2'4", "WT 15.2LB", "NO. 001",
        { "A STRANGE SEED WAS", "PLANTED ON ITS", "BACK AT BIRTH." },
        { "THE PLANT SPROUTS", "AND GROWS WITH", "THIS POKEMON" },
        g_pokedex_bulbasaur_tiles,
        11,
    },
    {
        "CHARMANDER", "LIZARD", "HT 2'0", "WT 19.0LB", "NO. 004",
        { "OBVIOUSLY PREFERS", "HOT PLACES. WHEN", "IT RAINS, STEAM" },
        { "IS SAID TO SPOUT", "FROM THE TIP OF", "ITS TAIL" },
        g_pokedex_charmander_tiles,
        12,
    },
    {
        "SQUIRTLE", "TURTLE", "HT 1'8", "WT 20.0LB", "NO. 007",
        { "AFTER BIRTH, ITS", "BACK SWELLS AND", "HARDENS INTO A" },
        { "SHELL. POWERFULLY", "SPRAYS FOAM FROM", "ITS MOUTH" },
        g_pokedex_squirtle_tiles,
        13,
    },
};

static bool8 s_open;
static u8 s_page;
static const PokedexEntry *s_entry;
static u16 s_saved_dispcnt;

static void pd_tile(u8 col, u8 row, u8 tile) {
    text_draw_tile_pal(col, row, tile, POKEDEX_PAL);
}

static void prepare_pokedex_palettes(void) {
    vu16 *paper = (vu16 *)MEM_PAL + POKEDEX_PAL * 16;
    vu16 *bulba = (vu16 *)MEM_PAL + 11 * 16;
    vu16 *charm = (vu16 *)MEM_PAL + 12 * 16;
    vu16 *squirt = (vu16 *)MEM_PAL + 13 * 16;

    for (u8 i = 0; i < 16; i++) {
        paper[i] = 0x7FFF;
        bulba[i] = 0x7FFF;
        charm[i] = 0x7FFF;
        squirt[i] = 0x7FFF;
    }

    paper[1] = RGB15(2, 1, 2);
    paper[2] = RGB15(31, 25, 31);
    paper[3] = RGB15(8, 18, 8);
    paper[4] = RGB15(20, 25, 20);

    bulba[1] = charm[1] = squirt[1] = RGB15(2, 1, 2);
    bulba[2] = charm[2] = squirt[2] = RGB15(31, 25, 31);
    bulba[3] = RGB15(5, 15, 4);
    bulba[4] = RGB15(18, 28, 12);
    charm[3] = RGB15(20, 5, 2);
    charm[4] = RGB15(31, 16, 4);
    squirt[3] = RGB15(3, 9, 22);
    squirt[4] = RGB15(13, 23, 31);
}
static u16 s_saved_palette[16];
static bool8 s_palette_saved;

static void draw_frame(void) {
    for (u8 row = 0; row < 20; row++)
        for (u8 col = 0; col < 30; col++)
            pd_tile(col, row, BOX_FILL);

    pd_tile(0, 0, BOX_TL);
    pd_tile(29, 0, BOX_TR);
    pd_tile(29, 19, BOX_BR);
    pd_tile(0, 19, BOX_BL);
    for (u8 col = 1; col < 29; col++) {
        pd_tile(col, 0, BOX_TE);
        pd_tile(col, 19, BOX_BE);
    }
    for (u8 row = 1; row < 19; row++) {
        pd_tile(0, row, BOX_LE);
        pd_tile(29, row, BOX_RE);
    }

    // The original screen has a divider below the metadata.
    for (u8 col = 1; col < 29; col++)
        pd_tile(col, 10, BOX_BE);
}

static void set_pokedex_palette(void) {
    vu16 *pal = (vu16 *)MEM_PAL + TEXT_PAL * 16;
    if (!s_palette_saved) {
        for (u8 i = 0; i < 16; i++) s_saved_palette[i] = pal[i];
        s_palette_saved = TRUE;
    }
    pal[1] = RGB15(2, 1, 2);       // ink / outline
    pal[2] = RGB15(31, 25, 31);    // pale Pokédex paper
    pal[3] = RGB15(10, 16, 10);    // dark sprite shade
    pal[4] = RGB15(21, 21, 21);    // light sprite shade
}

static void draw_description(const PokedexEntry *entry) {
    for (u8 row = 12; row <= 16; row++)
        for (u8 col = 1; col < 29; col++)
            pd_tile(col, row, BOX_FILL);

    const char *const *lines = (s_page == 0) ? entry->description
                                              : entry->description_page2;
    for (u8 i = 0; i < 3; i++)
        text_draw_str_pal(1, (u8)(12 + i * 2), lines[i], POKEDEX_PAL);
}

static void load_pokemon_picture(const u32 *tiles, u8 palette) {
    for (u32 i = 0; i < 200; i++)
        POKEDEX_TILE_VRAM[(POKEDEX_SPRITE_TILE * 8) + i] = tiles[i];

    for (u8 row = 0; row < 5; row++)
        for (u8 col = 0; col < 5; col++)
            text_draw_tile_pal((u8)(2 + col), (u8)(3 + row),
                               (u8)(POKEDEX_SPRITE_TILE + row * 5 + col),
                               palette);
}

void pokedex_open(PokedexSpecies species) {
    if (species >= ARRAY_COUNT(s_entries)) species = POKEDEX_BULBASAUR;
    const PokedexEntry *entry = &s_entries[species];
    s_entry = entry;
    s_page = 0;
    if (species == POKEDEX_BULBASAUR)
        audio_sfx_play(AUDIO_SFX_CRY_BULBASAUR);
    else if (species == POKEDEX_CHARMANDER)
        audio_sfx_play(AUDIO_SFX_CRY_CHARMANDER);
    else if (species == POKEDEX_SQUIRTLE)
        audio_sfx_play(AUDIO_SFX_CRY_SQUIRTLE);

    text_clear();
    prepare_pokedex_palettes();
    s_saved_dispcnt = REG_DISPCNT;
    REG_DISPCNT = (u16)((s_saved_dispcnt & (u16)~(DCNT_BG1 | DCNT_BG2 | DCNT_OBJ)) |
                        DCNT_BG0);
    draw_frame();
    load_pokemon_picture(entry->tiles, entry->sprite_palette);

    text_draw_str_pal(9, 2, entry->name, POKEDEX_PAL);
    text_draw_str_pal(9, 4, entry->category, POKEDEX_PAL);
    text_draw_str_pal(9, 6, entry->height, POKEDEX_PAL);
    text_draw_str_pal(15, 8, entry->weight, POKEDEX_PAL);
    text_draw_str_pal(2, 8, entry->number, POKEDEX_PAL);
    draw_description(entry);
    s_open = TRUE;
}

bool8 pokedex_update(void) {
    if (!s_open) return TRUE;
    if (!input_pressed(KEY_A) && !input_pressed(KEY_B)) return FALSE;
    if (s_page == 0) {
        s_page = 1;
        draw_description(s_entry);
        return FALSE;
    }
    return TRUE;
}

void pokedex_close(void) {
    if (!s_open) return;
    s_open = FALSE;
    text_init();
    tilemap_rebuild();
    tilemap_update_scroll();
    REG_DISPCNT = s_saved_dispcnt;
    if (s_palette_saved) {
        vu16 *pal = (vu16 *)MEM_PAL + TEXT_PAL * 16;
        for (u8 i = 0; i < 16; i++) pal[i] = s_saved_palette[i];
        s_palette_saved = FALSE;
    }
}
