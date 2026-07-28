#include "pokedex.h"
#include "dialog.h"
#include "gba.h"
#include "gfx_pokedex.h"
#include "input.h"
#include "text.h"
#include "world.h"
#include "audio.h"
#include "save.h"
#include "pokemon_palette.h"

#define POKEDEX_SPRITE_TILE 140
#define POKEDEX_PAL 14
#define POKEDEX_TILE_VRAM ((u32 *)(MEM_VRAM + 0x4000))

#if 0
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
    [POKEDEX_BULBASAUR] = {
        "BULBASAUR", "SEED", "HT 2'4", "WT 15.2LB", "NO. 001",
        { "A STRANGE SEED WAS", "PLANTED ON ITS", "BACK AT BIRTH." },
        { "THE PLANT SPROUTS", "AND GROWS WITH", "THIS POKEMON" },
        g_pokedex_bulbasaur_tiles,
        11,
    },
    [POKEDEX_CHARMANDER] = {
        "CHARMANDER", "LIZARD", "HT 2'0", "WT 19.0LB", "NO. 004",
        { "OBVIOUSLY PREFERS", "HOT PLACES. WHEN", "IT RAINS, STEAM" },
        { "IS SAID TO SPOUT", "FROM THE TIP OF", "ITS TAIL" },
        g_pokedex_charmander_tiles,
        12,
    },
    [POKEDEX_SQUIRTLE] = {
        "SQUIRTLE", "TURTLE", "HT 1'8", "WT 20.0LB", "NO. 007",
        { "AFTER BIRTH, ITS", "BACK SWELLS AND", "HARDENS INTO A" },
        { "SHELL. POWERFULLY", "SPRAYS FOAM FROM", "ITS MOUTH" },
        g_pokedex_squirtle_tiles,
        13,
    },
    [POKEDEX_PIDGEY] = {
        "PIDGEY", "TINY BIRD", "HT 1'0", "WT 4.0LB", "NO. 016",
        { "A COMMON SIGHT IN", "FORESTS AND WOODS.", "IT FLAPS ITS WINGS" },
        { "AT GROUND LEVEL TO", "KICK UP BLINDING", "SAND." },
        g_pokedex_pidgey_tiles,
        8,
    },
    [POKEDEX_RATTATA] = {
        "RATTATA", "RAT", "HT 1'0", "WT 7.7LB", "NO. 019",
        { "BITES ANYTHING WHEN", "IT ATTACKS. SMALL", "AND VERY QUICK, IT" },
        { "IS A COMMON SIGHT", "IN MANY PLACES.", "" },
        g_pokedex_rattata_tiles,
        9,
    },
    [POKEDEX_NIDORINO] = {
        "NIDORINO", "POISON PIN", "HT 2'11", "WT 43.0LB", "NO. 033",
        { "AN AGGRESSIVE TYPE", "THAT IS QUICK TO", "ATTACK." },
        { "ITS HORN SECRETES", "A POWERFUL VENOM", "WHEN IT STABS." },
        g_pokedex_nidorino_tiles,
        4,
    },
    [POKEDEX_SPEAROW] = {
        "SPEAROW", "TINY BIRD", "HT 1'0", "WT 4.4LB", "NO. 021",
        { "EATS BUGS IN GRASSY", "AREAS. IT FLAPS", "ITS WINGS FAST." },
        { "INEPT AT FLYING", "HIGH. CANNOT STAND", "COLD WEATHER." },
        g_pokedex_spearow_tiles,
        5,
    },
    [POKEDEX_NIDORAN_F] = {
        "NIDORAN F", "POISON PIN", "HT 1'4", "WT 15.4LB", "NO. 029",
        { "ALTHOUGH SMALL,", "VENOMOUS BARBS ON", "ITS BACK ARE SHARP." },
        { "THE FEMALE HAS", "SMALLER HORNS THAN", "THE MALE." },
        g_pokedex_nidoran_f_tiles,
        6,
    },
    [POKEDEX_NIDORAN_M] = {
        "NIDORAN M", "POISON PIN", "HT 1'8", "WT 19.8LB", "NO. 032",
        { "VERY PROTECTIVE OF", "ITS TERRITORY.", "ATTACKS WITH POISON" },
        { "SPIKES IF ANYTHING", "GETS TOO CLOSE.", "" },
        g_pokedex_nidoran_m_tiles,
        7,
    },
    [POKEDEX_WEEDLE] = {
        "WEEDLE", "HAIRY BUG", "HT 1'0", "WT 7.0LB", "NO. 013",
        { "OFTEN FOUND IN", "FORESTS, EATING", "LEAVES." },
        { "IT HAS A SHARP", "VENOMOUS STINGER", "ON ITS HEAD." },
        g_pokedex_weedle_tiles,
        2,
    },
};
#endif

static bool8 s_open;
static u8 s_page;
static const PokedexEntry *s_entry;
static u16 s_saved_dispcnt;

static void pd_tile(u8 col, u8 row, u8 tile) {
    text_draw_tile_pal(col, row, tile, POKEDEX_PAL);
}

static void prepare_pokedex_palettes(void) {
    vu16 *paper   = (vu16 *)MEM_PAL + POKEDEX_PAL * 16;
    vu16 *bulba   = (vu16 *)MEM_PAL + 11 * 16;
    vu16 *charm   = (vu16 *)MEM_PAL + 12 * 16;
    vu16 *squirt  = (vu16 *)MEM_PAL + 13 * 16;
    vu16 *pidgey  = (vu16 *)MEM_PAL + 8 * 16;
    vu16 *rattata = (vu16 *)MEM_PAL + 9 * 16;
    vu16 *nidorino = (vu16 *)MEM_PAL + 4 * 16;
    vu16 *spearow  = (vu16 *)MEM_PAL + 5 * 16;
    vu16 *nidof    = (vu16 *)MEM_PAL + 6 * 16;
    vu16 *nidom    = (vu16 *)MEM_PAL + 7 * 16;
    vu16 *weedle   = (vu16 *)MEM_PAL + 2 * 16;

    for (u8 i = 0; i < 16; i++) {
        paper[i] = 0x7FFF;
        bulba[i] = 0x7FFF;
        charm[i] = 0x7FFF;
        squirt[i] = 0x7FFF;
        pidgey[i] = 0x7FFF;
        rattata[i] = 0x7FFF;
        nidorino[i] = 0x7FFF;
        spearow[i] = 0x7FFF;
        nidof[i] = 0x7FFF;
        nidom[i] = 0x7FFF;
        weedle[i] = 0x7FFF;
    }

    paper[1] = RGB15(2, 1, 2);
    paper[2] = RGB15(31, 31, 31);
    paper[3] = RGB15(8, 18, 8);
    paper[4] = RGB15(20, 25, 20);

    bulba[1] = charm[1] = squirt[1] = RGB15(2, 1, 2);
    bulba[2] = charm[2] = squirt[2] = RGB15(31, 31, 31);
    bulba[3] = RGB15(5, 15, 4);
    bulba[4] = RGB15(18, 28, 12);
    charm[3] = RGB15(20, 5, 2);
    charm[4] = RGB15(31, 16, 4);
    squirt[3] = RGB15(3, 9, 22);
    squirt[4] = RGB15(13, 23, 31);

    pidgey[1] = rattata[1] = RGB15(2, 1, 2);
    pidgey[2] = rattata[2] = RGB15(31, 31, 31);
    pidgey[3] = RGB15(16, 12, 6);
    pidgey[4] = RGB15(26, 22, 14);
    rattata[3] = RGB15(14, 8, 16);
    rattata[4] = RGB15(24, 18, 26);

    nidof[1] = nidom[1] = nidorino[1] = spearow[1] = weedle[1] = RGB15(2, 1, 2);
    nidof[2] = nidom[2] = nidorino[2] = spearow[2] = weedle[2] = RGB15(31, 31, 31);
    nidof[3]    = RGB15(18, 12, 20);
    nidof[4]    = RGB15(31, 22, 29);
    nidom[3]    = RGB15(12, 10, 20);
    nidom[4]    = RGB15(23, 20, 31);
    nidorino[3] = RGB15(12, 10, 20);
    nidorino[4] = RGB15(23, 20, 31);
    spearow[3]  = RGB15(20, 10, 6);
    spearow[4]  = RGB15(31, 22, 14);
    weedle[3]   = RGB15(20, 14, 4);
    weedle[4]   = RGB15(31, 28, 14);
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

bool8 pokedex_species_to_entry(PokemonId species, PokedexSpecies *out) {
    if (!out || species < MON_BULBASAUR || species > NUM_POKEMON)
        return FALSE;
    *out = species;
    return TRUE;
}

void pokedex_open(PokedexSpecies species) {
    if (species < MON_BULBASAUR || species > NUM_POKEMON)
        species = MON_BULBASAUR;
    const PokedexEntry *entry = &g_pokedex_entries[species];
    s_entry = entry;
    s_page = 0;
    if (species == POKEDEX_BULBASAUR)
        audio_sfx_play(AUDIO_SFX_CRY_BULBASAUR);
    else if (species == POKEDEX_CHARMANDER)
        audio_sfx_play(AUDIO_SFX_CRY_CHARMANDER);
    else if (species == POKEDEX_SQUIRTLE)
        audio_sfx_play(AUDIO_SFX_CRY_SQUIRTLE);
    else
        audio_sfx_play(AUDIO_SFX_CRY_WILD);

    text_clear();
    prepare_pokedex_palettes();
    pokemon_palette_load((vu16 *)MEM_PAL + POKEDEX_PAL * 16, species);
    s_saved_dispcnt = REG_DISPCNT;
    REG_DISPCNT = (u16)((s_saved_dispcnt & (u16)~(DCNT_BG1 | DCNT_BG2 | DCNT_OBJ)) |
                        DCNT_BG0);
    draw_frame();
    load_pokemon_picture(pokedex_sprite_tiles(species), POKEDEX_PAL);

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

// ─── Pokédex seen/owned tracking ─────────────────────────────────────────────

static u8 s_dex_seen[POKEDEX_BYTES];
static u8 s_dex_owned[POKEDEX_BYTES];

void pokedex_tracking_clear(void) {
    for (u8 i = 0; i < POKEDEX_BYTES; i++) {
        s_dex_seen[i] = 0;
        s_dex_owned[i] = 0;
    }
}

void pokedex_set_seen(PokemonId species) {
    if (species < 1 || species > NUM_POKEMON) return;
    u8 idx = (u8)(species - 1);
    s_dex_seen[idx / 8] |= (u8)(1 << (idx % 8));
}

void pokedex_set_owned(PokemonId species) {
    if (species < 1 || species > NUM_POKEMON) return;
    u8 idx = (u8)(species - 1);
    s_dex_owned[idx / 8] |= (u8)(1 << (idx % 8));
    s_dex_seen[idx / 8] |= (u8)(1 << (idx % 8));
}

bool8 pokedex_is_seen(PokemonId species) {
    if (species < 1 || species > NUM_POKEMON) return FALSE;
    u8 idx = (u8)(species - 1);
    return (s_dex_seen[idx / 8] >> (idx % 8)) & 1;
}

bool8 pokedex_is_owned(PokemonId species) {
    if (species < 1 || species > NUM_POKEMON) return FALSE;
    u8 idx = (u8)(species - 1);
    return (s_dex_owned[idx / 8] >> (idx % 8)) & 1;
}

u16 pokedex_seen_count(void) {
    u16 count = 0;
    for (u8 i = 0; i < POKEDEX_BYTES; i++) {
        u8 b = s_dex_seen[i];
        while (b) { count++; b &= (u8)(b - 1); }
    }
    return count;
}

u16 pokedex_owned_count(void) {
    u16 count = 0;
    for (u8 i = 0; i < POKEDEX_BYTES; i++) {
        u8 b = s_dex_owned[i];
        while (b) { count++; b &= (u8)(b - 1); }
    }
    return count;
}

void pokedex_tracking_export(u8 seen[20], u8 owned[20]) {
    for (u8 i = 0; i < POKEDEX_BYTES; i++) {
        seen[i] = s_dex_seen[i];
        owned[i] = s_dex_owned[i];
    }
}

void pokedex_tracking_import(const u8 seen[20], const u8 owned[20]) {
    for (u8 i = 0; i < POKEDEX_BYTES; i++) {
        s_dex_seen[i] = seen[i];
        s_dex_owned[i] = owned[i];
    }
}

// ─── Pokédex list menu ───────────────────────────────────────────────────────

static const char *s_species_names[] = {
    "", "BULBASAUR", "IVYSAUR", "VENUSAUR",
    "CHARMANDER", "CHARMELEON", "CHARIZARD",
    "SQUIRTLE", "WARTORTLE", "BLASTOISE",
    "CATERPIE", "METAPOD", "BUTTERFREE",
    "WEEDLE", "KAKUNA", "BEEDRILL",
    "PIDGEY", "PIDGEOTTO", "PIDGEOT",
    "RATTATA", "RATICATE", "SPEAROW",
    "FEAROW", "EKANS", "ARBOK",
    "PIKACHU", "RAICHU", "SANDSHREW",
    "SANDSLASH", "NIDORAN F", "NIDORINA",
    "NIDOQUEEN", "NIDORAN M", "NIDORINO",
    "NIDOKING", "CLEFAIRY", "CLEFABLE",
    "VULPIX", "NINETALES", "JIGGLYPUFF",
    "WIGGLYTUFF", "ZUBAT", "GOLBAT",
    "ODDISH", "GLOOM", "VILEPLUME",
    "PARAS", "PARASECT", "VENONAT",
    "VENOMOTH", "DIGLETT", "DUGTRIO",
    "MEOWTH", "PERSIAN", "PSYDUCK",
    "GOLDUCK", "MANKEY", "PRIMEAPE",
    "GROWLITHE", "ARCANINE", "POLIWAG",
    "POLIWHIRL", "POLIWRATH", "ABRA",
    "KADABRA", "ALAKAZAM", "MACHOP",
    "MACHOKE", "MACHAMP", "BELLSPROUT",
    "WEEPINBELL", "VICTREEBEL", "TENTACOOL",
    "TENTACRUEL", "GEODUDE", "GRAVELER",
    "GOLEM", "PONYTA", "RAPIDASH",
    "SLOWPOKE", "SLOWBRO", "MAGNEMITE",
    "MAGNETON", "FARFETCH'D", "DODUO",
    "DODRIO", "SEEL", "DEWGONG",
    "GRIMER", "MUK", "SHELLDER",
    "CLOYSTER", "GASTLY", "HAUNTER",
    "GENGAR", "ONIX", "DROWZEE",
    "HYPNO", "KRABBY", "KINGLER",
    "VOLTORB", "ELECTRODE", "EXEGGCUTE",
    "EXEGGUTOR", "CUBONE", "MAROWAK",
    "HITMONLEE", "HITMONCHAN", "LICKITUNG",
    "KOFFING", "WEEZING", "RHYHORN",
    "RHYDON", "CHANSEY", "TANGELA",
    "KANGASKHAN", "HORSEA", "SEADRA",
    "GOLDEEN", "SEAKING", "STARYU",
    "STARMIE", "MR.MIME", "SCYTHER",
    "JYNX", "ELECTABUZZ", "MAGMAR",
    "PINSIR", "TAUROS", "MAGIKARP",
    "GYARADOS", "LAPRAS", "DITTO",
    "EEVEE", "VAPOREON", "JOLTEON",
    "FLAREON", "PORYGON", "OMANYTE",
    "OMASTAR", "KABUTO", "KABUTOPS",
    "AERODACTYL", "SNORLAX", "ARTICUNO",
    "ZAPDOS", "MOLTRES", "DRATINI",
    "DRAGONAIR", "DRAGONITE", "MEWTWO",
    "MEW",
};

static bool8 s_list_open;
static u8 s_list_cursor;
static u8 s_list_scroll;
static u16 s_list_saved_dispcnt;
static bool8 s_list_detail_open;

static void dex_draw_box(u8 left, u8 top, u8 right, u8 bottom) {
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
            text_draw_tile(x, y, tile);
        }
    }
}

static void pokedex_list_draw(void) {
    text_clear();
    dex_draw_box(0, 0, 29, 19);

    u16 seen = pokedex_seen_count();
    u16 owned = pokedex_owned_count();
    char buf[20];

    text_draw_str(1, 1, "POKeDEX");
    char *p = buf;
    p[0] = 'S'; p[1] = 'E'; p[2] = 'E'; p[3] = 'N'; p[4] = ' ';
    p += 5;
    if (seen >= 100) { *p++ = (char)('0' + seen / 100); }
    if (seen >= 10) { *p++ = (char)('0' + (seen / 10) % 10); }
    *p++ = (char)('0' + seen % 10);
    *p = '\0';
    text_draw_str(12, 1, buf);

    p = buf;
    p[0] = 'O'; p[1] = 'W'; p[2] = 'N'; p[3] = ' ';
    p += 4;
    if (owned >= 100) { *p++ = (char)('0' + owned / 100); }
    if (owned >= 10) { *p++ = (char)('0' + (owned / 10) % 10); }
    *p++ = (char)('0' + owned % 10);
    *p = '\0';
    text_draw_str(21, 1, buf);

    for (u8 i = 0; i < 8; i++) {
        u8 dex_num = (u8)(s_list_scroll + i + 1);
        u8 row = (u8)(3 + i * 2);

        for (u8 col = 1; col < 29; col++)
            text_draw_char(col, row, ' ');

        if (dex_num > NUM_POKEMON) continue;

        char num[5];
        num[0] = (char)('0' + dex_num / 100);
        num[1] = (char)('0' + (dex_num / 10) % 10);
        num[2] = (char)('0' + dex_num % 10);
        num[3] = '\0';
        text_draw_str(2, row, num);

        if (pokedex_is_seen((PokemonId)dex_num)) {
            text_draw_str(6, row, s_species_names[dex_num]);
            if (pokedex_is_owned((PokemonId)dex_num))
                text_draw_char(1, row, '*');
        } else {
            text_draw_str(6, row, "----------");
        }

        if (i == s_list_cursor)
            text_draw_char(1, row, '>');
    }
}

void pokedex_list_open(void) {
    s_list_cursor = 0;
    s_list_scroll = 0;
    s_list_open = TRUE;
    s_list_detail_open = FALSE;
    s_list_saved_dispcnt = REG_DISPCNT;
    REG_DISPCNT = (u16)((s_list_saved_dispcnt & (u16)~(DCNT_BG1 | DCNT_BG2 | DCNT_OBJ)) |
                        DCNT_BG0);
    pokedex_list_draw();
}

bool8 pokedex_list_update(void) {
    if (!s_list_open) return TRUE;

    if (s_list_detail_open) {
        if (pokedex_update()) {
            pokedex_close();
            s_list_detail_open = FALSE;
            REG_DISPCNT = (u16)((s_list_saved_dispcnt & (u16)~(DCNT_BG1 | DCNT_BG2 | DCNT_OBJ)) |
                                DCNT_BG0);
            pokedex_list_draw();
        }
        return FALSE;
    }

    if (input_pressed(KEY_B)) {
        return TRUE;
    }

    bool8 redraw = FALSE;
    if (input_pressed(KEY_UP)) {
        if (s_list_cursor > 0) {
            s_list_cursor--;
        } else if (s_list_scroll > 0) {
            s_list_scroll--;
        }
        redraw = TRUE;
        audio_sfx_play(AUDIO_SFX_SELECT);
    }
    if (input_pressed(KEY_DOWN)) {
        u8 dex_num = (u8)(s_list_scroll + s_list_cursor + 1);
        if (dex_num < NUM_POKEMON) {
            if (s_list_cursor < 7) {
                s_list_cursor++;
            } else {
                s_list_scroll++;
            }
            redraw = TRUE;
            audio_sfx_play(AUDIO_SFX_SELECT);
        }
    }

    if (input_pressed(KEY_A)) {
        u8 dex_num = (u8)(s_list_scroll + s_list_cursor + 1);
        if (dex_num <= NUM_POKEMON && pokedex_is_owned((PokemonId)dex_num)) {
            PokedexSpecies sp;
            if (pokedex_species_to_entry((PokemonId)dex_num, &sp)) {
                pokedex_open(sp);
                s_list_detail_open = TRUE;
                audio_sfx_play(AUDIO_SFX_CONFIRM);
            } else {
                audio_sfx_play(AUDIO_SFX_CONFIRM);
            }
            return FALSE;
        }
    }

    if (redraw) pokedex_list_draw();
    return FALSE;
}

void pokedex_list_close(void) {
    if (!s_list_open) return;
    s_list_open = FALSE;
    text_init();
    tilemap_rebuild();
    tilemap_update_scroll();
    REG_DISPCNT = s_list_saved_dispcnt;
}
