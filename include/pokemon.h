#pragma once
#include "types.h"

// ── Types ─────────────────────────────────────────────────────────────────────
typedef enum {
    TYPE_NORMAL    = 0x00,
    TYPE_FIGHTING  = 0x01,
    TYPE_FLYING    = 0x02,
    TYPE_POISON    = 0x03,
    TYPE_GROUND    = 0x04,
    TYPE_ROCK      = 0x05,
    TYPE_BIRD      = 0x06,  // unused in Gen 1
    TYPE_BUG       = 0x07,
    TYPE_GHOST     = 0x08,
    // 0x09-0x13 unused
    TYPE_FIRE      = 0x14,
    TYPE_WATER     = 0x15,
    TYPE_GRASS     = 0x16,
    TYPE_ELECTRIC  = 0x17,
    TYPE_PSYCHIC   = 0x18,
    TYPE_ICE       = 0x19,
    TYPE_DRAGON    = 0x1A,
} PokemonType;

// ── Growth rates ──────────────────────────────────────────────────────────────
typedef enum {
    GROWTH_MEDIUM_FAST   = 0,
    GROWTH_IRREGULAR     = 1,
    GROWTH_FLUCTUATING   = 2,
    GROWTH_MEDIUM_SLOW   = 3,
    GROWTH_FAST          = 4,
    GROWTH_SLOW          = 5,
} GrowthRate;

// ── Base stats ────────────────────────────────────────────────────────────────
typedef struct {
    u8 hp;
    u8 attack;
    u8 defense;
    u8 speed;
    u8 special;
    PokemonType type1;
    PokemonType type2;
    u8 catch_rate;
    u8 base_exp;
} PokemonBaseStats;

#define NUM_POKEMON 151

extern const PokemonBaseStats g_pokemon_base_stats[NUM_POKEMON + 1];

// ── Pokémon IDs (Pokédex order, 1-indexed — index 0 is unused) ───────────────
typedef enum {
    MON_NONE = 0,
    MON_BULBASAUR = 1,
    MON_IVYSAUR,
    MON_VENUSAUR,
    MON_CHARMANDER,
    MON_CHARMELEON,
    MON_CHARIZARD,
    MON_SQUIRTLE,
    MON_WARTORTLE,
    MON_BLASTOISE,
    MON_CATERPIE,    // 10
    MON_METAPOD,
    MON_BUTTERFREE,
    MON_WEEDLE,
    MON_KAKUNA,
    MON_BEEDRILL,
    MON_PIDGEY,
    MON_PIDGEOTTO,
    MON_PIDGEOT,
    MON_RATTATA,
    MON_RATICATE,    // 20
    MON_SPEAROW,
    MON_FEAROW,
    MON_EKANS,
    MON_ARBOK,
    MON_PIKACHU,
    MON_RAICHU,
    MON_SANDSHREW,
    MON_SANDSLASH,
    MON_NIDORAN_F,
    MON_NIDORINA,    // 30
    MON_NIDOQUEEN,
    MON_NIDORAN_M,
    MON_NIDORINO,
    MON_NIDOKING,
    MON_CLEFAIRY,
    MON_CLEFABLE,
    MON_VULPIX,
    MON_NINETALES,
    MON_JIGGLYPUFF,
    MON_WIGGLYTUFF,  // 40
    MON_ZUBAT,
    MON_GOLBAT,
    MON_ODDISH,
    MON_GLOOM,
    MON_VILEPLUME,
    MON_PARAS,
    MON_PARASECT,
    MON_VENONAT,
    MON_VENOMOTH,
    MON_DIGLETT,     // 50
    MON_DUGTRIO,
    MON_MEOWTH,
    MON_PERSIAN,
    MON_PSYDUCK,
    MON_GOLDUCK,
    MON_MANKEY,
    MON_PRIMEAPE,
    MON_GROWLITHE,
    MON_ARCANINE,
    MON_POLIWAG,     // 60
    MON_POLIWHIRL,
    MON_POLIWRATH,
    MON_ABRA,
    MON_KADABRA,
    MON_ALAKAZAM,
    MON_MACHOP,
    MON_MACHOKE,
    MON_MACHAMP,
    MON_BELLSPROUT,
    MON_WEEPINBELL,  // 70
    MON_VICTREEBEL,
    MON_TENTACOOL,
    MON_TENTACRUEL,
    MON_GEODUDE,
    MON_GRAVELER,
    MON_GOLEM,
    MON_PONYTA,
    MON_RAPIDASH,
    MON_SLOWPOKE,
    MON_SLOWBRO,     // 80
    MON_MAGNEMITE,
    MON_MAGNETON,
    MON_FARFETCHD,
    MON_DODUO,
    MON_DODRIO,
    MON_SEEL,
    MON_DEWGONG,
    MON_GRIMER,
    MON_MUK,
    MON_SHELLDER,    // 90
    MON_CLOYSTER,
    MON_GASTLY,
    MON_HAUNTER,
    MON_GENGAR,
    MON_ONIX,
    MON_DROWZEE,
    MON_HYPNO,
    MON_KRABBY,
    MON_KINGLER,
    MON_VOLTORB,     // 100
    MON_ELECTRODE,
    MON_EXEGGCUTE,
    MON_EXEGGUTOR,
    MON_CUBONE,
    MON_MAROWAK,
    MON_HITMONLEE,
    MON_HITMONCHAN,
    MON_LICKITUNG,
    MON_KOFFING,
    MON_WEEZING,     // 110
    MON_RHYHORN,
    MON_RHYDON,
    MON_CHANSEY,
    MON_TANGELA,
    MON_KANGASKHAN,
    MON_HORSEA,
    MON_SEADRA,
    MON_GOLDEEN,
    MON_SEAKING,
    MON_STARYU,      // 120
    MON_STARMIE,
    MON_MR_MIME,
    MON_SCYTHER,
    MON_JYNX,
    MON_ELECTABUZZ,
    MON_MAGMAR,
    MON_PINSIR,
    MON_TAUROS,
    MON_MAGIKARP,
    MON_GYARADOS,    // 130
    MON_LAPRAS,
    MON_DITTO,
    MON_EEVEE,
    MON_VAPOREON,
    MON_JOLTEON,
    MON_FLAREON,
    MON_PORYGON,
    MON_OMANYTE,
    MON_OMASTAR,
    MON_KABUTO,      // 140
    MON_KABUTOPS,
    MON_AERODACTYL,
    MON_SNORLAX,
    MON_ARTICUNO,
    MON_ZAPDOS,
    MON_MOLTRES,
    MON_DRATINI,
    MON_DRAGONAIR,
    MON_DRAGONITE,
    MON_MEWTWO,      // 150
    MON_MEW,         // 151
} PokemonId;
