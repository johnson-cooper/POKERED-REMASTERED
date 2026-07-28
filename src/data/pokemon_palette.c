#include "pokemon_palette.h"
#include "pokemon.h"

typedef struct {
    u16 dark;
    u16 light;
} SpeciesPalette;

static const SpeciesPalette s_type_palettes[] = {
    [TYPE_NORMAL]   = { RGB15(15, 15, 15), RGB15(27, 27, 27) },
    [TYPE_FIGHTING] = { RGB15(18, 8, 5),   RGB15(29, 17, 10) },
    [TYPE_FLYING]   = { RGB15(16, 12, 6),  RGB15(26, 22, 14) },
    [TYPE_POISON]   = { RGB15(12, 10, 20), RGB15(23, 20, 31) },
    [TYPE_GROUND]   = { RGB15(18, 12, 5),  RGB15(29, 23, 12) },
    [TYPE_ROCK]     = { RGB15(13, 13, 13), RGB15(24, 24, 24) },
    [TYPE_BUG]      = { RGB15(20, 14, 4),  RGB15(31, 28, 14) },
    [TYPE_GHOST]    = { RGB15(12, 7, 18),  RGB15(23, 15, 29) },
    [TYPE_FIRE]     = { RGB15(20, 5, 2),   RGB15(31, 16, 4) },
    [TYPE_WATER]    = { RGB15(3, 9, 22),   RGB15(13, 23, 31) },
    [TYPE_GRASS]    = { RGB15(5, 15, 4),   RGB15(18, 28, 12) },
    [TYPE_ELECTRIC] = { RGB15(20, 16, 2),  RGB15(31, 29, 10) },
    [TYPE_PSYCHIC]  = { RGB15(20, 7, 15),  RGB15(31, 17, 27) },
    [TYPE_ICE]      = { RGB15(9, 16, 22),  RGB15(20, 30, 31) },
    [TYPE_DRAGON]   = { RGB15(7, 10, 20),  RGB15(16, 23, 31) },
};

static const SpeciesPalette s_bulbasaur_palette = { RGB15(5, 15, 4), RGB15(18, 28, 12) };
static const SpeciesPalette s_charmander_palette = { RGB15(20, 5, 2), RGB15(31, 16, 4) };
static const SpeciesPalette s_squirtle_palette = { RGB15(3, 9, 22), RGB15(13, 23, 31) };
static const SpeciesPalette s_pidgey_palette = { RGB15(18, 12, 8), RGB15(29, 24, 16) };
static const SpeciesPalette s_rattata_palette = { RGB15(18, 9, 18), RGB15(29, 18, 27) };
static const SpeciesPalette s_nidorino_palette = { RGB15(12, 10, 20), RGB15(23, 20, 31) };
static const SpeciesPalette s_nidoran_f_palette = { RGB15(18, 12, 20), RGB15(31, 22, 29) };
static const SpeciesPalette s_spearow_palette = { RGB15(20, 10, 6), RGB15(31, 22, 14) };
static const SpeciesPalette s_weedle_palette = { RGB15(20, 14, 4), RGB15(31, 28, 14) };

static const SpeciesPalette *palette_for_species(
    PokemonId species, const SpeciesPalette *fallback) {
    switch (species) {
    case MON_BULBASAUR: return &s_bulbasaur_palette;
    case MON_CHARMANDER:
    case MON_CHARMELEON: return &s_charmander_palette;
    case MON_SQUIRTLE: return &s_squirtle_palette;
    case MON_PIDGEY: return &s_pidgey_palette;
    case MON_RATTATA: return &s_rattata_palette;
    case MON_NIDORINO:
    case MON_NIDORAN_M: return &s_nidorino_palette;
    case MON_NIDORAN_F: return &s_nidoran_f_palette;
    case MON_SPEAROW: return &s_spearow_palette;
    case MON_WEEDLE: return &s_weedle_palette;
    default: return fallback;
    }
}

static void palette_load(vu16 *dst, PokemonId species, bool8 battle_layout) {
    if (species > NUM_POKEMON) species = MON_NONE;
    const PokemonBaseStats *base = &g_pokemon_base_stats[species];
    const SpeciesPalette *palette = &s_type_palettes[base->type1];
    palette = palette_for_species(species, palette);

    for (u8 i = 0; i < 16; i++) dst[i] = 0x7FFF;
    dst[0] = 0;
    dst[1] = RGB15(2, 1, 2);
    dst[2] = battle_layout ? palette->light : RGB15(31, 31, 31);
    dst[3] = palette->dark;
    dst[4] = palette->light;
}

void pokemon_palette_load(vu16 *dst, PokemonId species) {
    palette_load(dst, species, FALSE);
}

void pokemon_battle_palette_load(vu16 *dst, PokemonId species) {
    palette_load(dst, species, TRUE);
}
