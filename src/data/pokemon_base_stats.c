// All 151 Pokémon base stats, ported from refs/pokered/data/pokemon/base_stats/
// Order matches Pokédex order (index 1 = Bulbasaur, index 151 = Mew).
// Index 0 is a placeholder (MON_NONE).
#include "pokemon.h"

const PokemonBaseStats g_pokemon_base_stats[NUM_POKEMON + 1] = {
    // [0] MON_NONE (unused placeholder)
    [0]  = {0},

    // ── Grass starters ────────────────────────────────────────────────────────
    [1]  = { 45,  49,  49,  45,  65, TYPE_GRASS,    TYPE_POISON,  45,  64 }, // Bulbasaur
    [2]  = { 60,  62,  63,  60,  80, TYPE_GRASS,    TYPE_POISON,  45, 141 }, // Ivysaur
    [3]  = { 80,  82,  83,  80, 100, TYPE_GRASS,    TYPE_POISON,  45, 208 }, // Venusaur

    // ── Fire starters ─────────────────────────────────────────────────────────
    [4]  = { 39,  52,  43,  65,  50, TYPE_FIRE,     TYPE_FIRE,    45,  65 }, // Charmander
    [5]  = { 58,  64,  58,  80,  65, TYPE_FIRE,     TYPE_FIRE,    45, 142 }, // Charmeleon
    [6]  = { 78,  84,  78, 100,  85, TYPE_FIRE,     TYPE_FLYING,  45, 209 }, // Charizard

    // ── Water starters ────────────────────────────────────────────────────────
    [7]  = { 44,  48,  65,  43,  50, TYPE_WATER,    TYPE_WATER,   45,  66 }, // Squirtle
    [8]  = { 59,  63,  80,  58,  65, TYPE_WATER,    TYPE_WATER,   45, 143 }, // Wartortle
    [9]  = { 79,  83, 100,  78,  85, TYPE_WATER,    TYPE_WATER,   45, 210 }, // Blastoise

    // ── Bug line 1 ────────────────────────────────────────────────────────────
    [10] = { 45,  30,  35,  45,  20, TYPE_BUG,      TYPE_BUG,    255,  53 }, // Caterpie
    [11] = { 50,  20,  55,  30,  25, TYPE_BUG,      TYPE_BUG,    120,  72 }, // Metapod
    [12] = { 60,  45,  50,  70,  80, TYPE_BUG,      TYPE_FLYING,  45, 160 }, // Butterfree

    // ── Bug line 2 ────────────────────────────────────────────────────────────
    [13] = { 40,  35,  30,  50,  20, TYPE_BUG,      TYPE_POISON, 255,  52 }, // Weedle
    [14] = { 45,  25,  50,  35,  25, TYPE_BUG,      TYPE_POISON, 120,  71 }, // Kakuna
    [15] = { 65,  80,  40,  75,  45, TYPE_BUG,      TYPE_POISON,  45, 159 }, // Beedrill

    // ── Pidgey line ───────────────────────────────────────────────────────────
    [16] = { 40,  45,  40,  56,  35, TYPE_NORMAL,   TYPE_FLYING, 255,  55 }, // Pidgey
    [17] = { 63,  60,  55,  71,  50, TYPE_NORMAL,   TYPE_FLYING, 120, 113 }, // Pidgeotto
    [18] = { 83,  80,  75,  91,  70, TYPE_NORMAL,   TYPE_FLYING,  45, 172 }, // Pidgeot

    // ── Rattata line ──────────────────────────────────────────────────────────
    [19] = { 30,  56,  35,  72,  25, TYPE_NORMAL,   TYPE_NORMAL, 255,  57 }, // Rattata
    [20] = { 55,  81,  60,  97,  50, TYPE_NORMAL,   TYPE_NORMAL,  90, 116 }, // Raticate

    // ── Spearow line ──────────────────────────────────────────────────────────
    [21] = { 40,  60,  30,  70,  31, TYPE_NORMAL,   TYPE_FLYING, 255,  58 }, // Spearow
    [22] = { 65,  90,  65, 100,  61, TYPE_NORMAL,   TYPE_FLYING,  90, 162 }, // Fearow

    // ── Ekans line ────────────────────────────────────────────────────────────
    [23] = { 35,  60,  44,  55,  40, TYPE_POISON,   TYPE_POISON, 255,  62 }, // Ekans
    [24] = { 60,  85,  69,  80,  65, TYPE_POISON,   TYPE_POISON,  90, 147 }, // Arbok

    // ── Pikachu line ──────────────────────────────────────────────────────────
    [25] = { 35,  55,  30,  90,  50, TYPE_ELECTRIC, TYPE_ELECTRIC,190, 82 }, // Pikachu
    [26] = { 60,  90,  55, 100,  90, TYPE_ELECTRIC, TYPE_ELECTRIC, 75,122 }, // Raichu

    // ── Sandshrew line ────────────────────────────────────────────────────────
    [27] = { 50,  75,  85,  40,  30, TYPE_GROUND,   TYPE_GROUND, 255,  93 }, // Sandshrew
    [28] = { 75, 100, 110,  65,  55, TYPE_GROUND,   TYPE_GROUND,  90, 163 }, // Sandslash

    // ── Nidoran F line ────────────────────────────────────────────────────────
    [29] = { 55,  47,  52,  41,  40, TYPE_POISON,   TYPE_POISON, 235,  59 }, // Nidoran F
    [30] = { 70,  62,  67,  56,  55, TYPE_POISON,   TYPE_POISON, 120, 117 }, // Nidorina
    [31] = { 90,  82,  87,  76,  75, TYPE_POISON,   TYPE_GROUND,  45, 194 }, // Nidoqueen

    // ── Nidoran M line ────────────────────────────────────────────────────────
    [32] = { 46,  57,  40,  50,  40, TYPE_POISON,   TYPE_POISON, 235,  60 }, // Nidoran M
    [33] = { 61,  72,  57,  65,  55, TYPE_POISON,   TYPE_POISON, 120, 118 }, // Nidorino
    [34] = { 81,  92,  77,  85,  75, TYPE_POISON,   TYPE_GROUND,  45, 195 }, // Nidoking

    // ── Clefairy line ─────────────────────────────────────────────────────────
    [35] = { 70,  45,  48,  35,  60, TYPE_NORMAL,   TYPE_NORMAL, 150,  68 }, // Clefairy
    [36] = { 95,  70,  73,  60,  85, TYPE_NORMAL,   TYPE_NORMAL,  25, 129 }, // Clefable

    // ── Vulpix line ───────────────────────────────────────────────────────────
    [37] = { 38,  41,  40,  65,  65, TYPE_FIRE,     TYPE_FIRE,   190,  63 }, // Vulpix
    [38] = { 73,  76,  75, 100, 100, TYPE_FIRE,     TYPE_FIRE,    75, 178 }, // Ninetales

    // ── Jigglypuff line ───────────────────────────────────────────────────────
    [39] = {115,  45,  20,  20,  25, TYPE_NORMAL,   TYPE_NORMAL, 170,  76 }, // Jigglypuff
    [40] = {140,  70,  45,  45,  50, TYPE_NORMAL,   TYPE_NORMAL,  50, 109 }, // Wigglytuff

    // ── Zubat line ────────────────────────────────────────────────────────────
    [41] = { 40,  45,  35,  55,  40, TYPE_POISON,   TYPE_FLYING, 255,  54 }, // Zubat
    [42] = { 75,  80,  70,  90,  75, TYPE_POISON,   TYPE_FLYING,  90, 171 }, // Golbat

    // ── Oddish line ───────────────────────────────────────────────────────────
    [43] = { 45,  50,  55,  30,  75, TYPE_GRASS,    TYPE_POISON, 255,  78 }, // Oddish
    [44] = { 60,  65,  70,  40,  85, TYPE_GRASS,    TYPE_POISON, 120, 132 }, // Gloom
    [45] = { 75,  80,  85,  50, 100, TYPE_GRASS,    TYPE_POISON,  45, 184 }, // Vileplume

    // ── Paras line ────────────────────────────────────────────────────────────
    [46] = { 35,  70,  55,  25,  55, TYPE_BUG,      TYPE_GRASS,  190,  70 }, // Paras
    [47] = { 60,  95,  80,  30,  80, TYPE_BUG,      TYPE_GRASS,   75, 128 }, // Parasect

    // ── Venonat line ──────────────────────────────────────────────────────────
    [48] = { 60,  55,  50,  45,  40, TYPE_BUG,      TYPE_POISON, 190,  75 }, // Venonat
    [49] = { 70,  65,  60,  90,  90, TYPE_BUG,      TYPE_POISON,  75, 138 }, // Venomoth

    // ── Diglett line ──────────────────────────────────────────────────────────
    [50] = { 10,  55,  25,  95,  45, TYPE_GROUND,   TYPE_GROUND, 255,  81 }, // Diglett
    [51] = { 35,  80,  50, 120,  70, TYPE_GROUND,   TYPE_GROUND,  50, 153 }, // Dugtrio

    // ── Meowth line ───────────────────────────────────────────────────────────
    [52] = { 40,  45,  35,  90,  40, TYPE_NORMAL,   TYPE_NORMAL, 255,  69 }, // Meowth
    [53] = { 65,  70,  60, 115,  65, TYPE_NORMAL,   TYPE_NORMAL,  90, 148 }, // Persian

    // ── Psyduck line ──────────────────────────────────────────────────────────
    [54] = { 50,  52,  48,  55,  50, TYPE_WATER,    TYPE_WATER,  190,  80 }, // Psyduck
    [55] = { 80,  82,  78,  85,  80, TYPE_WATER,    TYPE_WATER,   75, 174 }, // Golduck

    // ── Mankey line ───────────────────────────────────────────────────────────
    [56] = { 40,  80,  35,  70,  35, TYPE_FIGHTING, TYPE_FIGHTING,190, 74 }, // Mankey
    [57] = { 65, 105,  60,  95,  60, TYPE_FIGHTING, TYPE_FIGHTING, 75,149 }, // Primeape

    // ── Growlithe line ────────────────────────────────────────────────────────
    [58] = { 55,  70,  45,  60,  50, TYPE_FIRE,     TYPE_FIRE,   190,  91 }, // Growlithe
    [59] = { 90, 110,  80,  95,  80, TYPE_FIRE,     TYPE_FIRE,    75, 213 }, // Arcanine

    // ── Poliwag line ──────────────────────────────────────────────────────────
    [60] = { 40,  50,  40,  90,  40, TYPE_WATER,    TYPE_WATER,  255,  77 }, // Poliwag
    [61] = { 65,  65,  65,  90,  50, TYPE_WATER,    TYPE_WATER,  120, 131 }, // Poliwhirl
    [62] = { 90,  85,  95,  70,  70, TYPE_WATER,    TYPE_FIGHTING, 45,185 }, // Poliwrath

    // ── Abra line ─────────────────────────────────────────────────────────────
    [63] = { 25,  20,  15,  90, 105, TYPE_PSYCHIC,  TYPE_PSYCHIC, 200,  73 }, // Abra
    [64] = { 40,  35,  30, 105, 120, TYPE_PSYCHIC,  TYPE_PSYCHIC, 100, 145 }, // Kadabra
    [65] = { 55,  50,  45, 120, 135, TYPE_PSYCHIC,  TYPE_PSYCHIC,  50, 186 }, // Alakazam

    // ── Machop line ───────────────────────────────────────────────────────────
    [66] = { 70,  80,  50,  35,  35, TYPE_FIGHTING, TYPE_FIGHTING,180, 88 }, // Machop
    [67] = { 80, 100,  70,  45,  50, TYPE_FIGHTING, TYPE_FIGHTING, 90,146 }, // Machoke
    [68] = { 90, 130,  80,  55,  65, TYPE_FIGHTING, TYPE_FIGHTING, 45,193 }, // Machamp

    // ── Bellsprout line ───────────────────────────────────────────────────────
    [69] = { 50,  75,  35,  40,  70, TYPE_GRASS,    TYPE_POISON, 255,  84 }, // Bellsprout
    [70] = { 65,  90,  50,  55,  85, TYPE_GRASS,    TYPE_POISON, 120, 151 }, // Weepinbell
    [71] = { 80, 105,  65,  70, 100, TYPE_GRASS,    TYPE_POISON,  45, 191 }, // Victreebel

    // ── Tentacool line ────────────────────────────────────────────────────────
    [72] = { 40,  40,  35,  70, 100, TYPE_WATER,    TYPE_POISON, 190, 105 }, // Tentacool
    [73] = { 80,  70,  65, 100, 120, TYPE_WATER,    TYPE_POISON,  60, 205 }, // Tentacruel

    // ── Geodude line ──────────────────────────────────────────────────────────
    [74] = { 40,  80, 100,  20,  30, TYPE_ROCK,     TYPE_GROUND, 255,  86 }, // Geodude
    [75] = { 55,  95, 115,  35,  45, TYPE_ROCK,     TYPE_GROUND, 120, 134 }, // Graveler
    [76] = { 80, 110, 130,  45,  55, TYPE_ROCK,     TYPE_GROUND,  45, 177 }, // Golem

    // ── Ponyta line ───────────────────────────────────────────────────────────
    [77] = { 50,  85,  55,  90,  65, TYPE_FIRE,     TYPE_FIRE,   190, 152 }, // Ponyta
    [78] = { 65, 100,  70, 105,  80, TYPE_FIRE,     TYPE_FIRE,    60, 192 }, // Rapidash

    // ── Slowpoke line ─────────────────────────────────────────────────────────
    [79] = { 90,  65,  65,  15,  40, TYPE_WATER,    TYPE_PSYCHIC, 190, 99 }, // Slowpoke
    [80] = { 95,  75, 110,  30,  80, TYPE_WATER,    TYPE_PSYCHIC,  75,164 }, // Slowbro

    // ── Magnemite line ────────────────────────────────────────────────────────
    [81] = { 25,  35,  70,  45,  95, TYPE_ELECTRIC, TYPE_ELECTRIC,190, 89 }, // Magnemite
    [82] = { 50,  60,  95,  70, 120, TYPE_ELECTRIC, TYPE_ELECTRIC, 60,161 }, // Magneton

    // ── Farfetch'd ────────────────────────────────────────────────────────────
    [83] = { 52,  65,  55,  60,  58, TYPE_NORMAL,   TYPE_FLYING,  45,  94 }, // Farfetch'd

    // ── Doduo line ────────────────────────────────────────────────────────────
    [84] = { 35,  85,  45,  75,  35, TYPE_NORMAL,   TYPE_FLYING, 190,  96 }, // Doduo
    [85] = { 60, 110,  70, 100,  60, TYPE_NORMAL,   TYPE_FLYING,  45, 158 }, // Dodrio

    // ── Seel line ─────────────────────────────────────────────────────────────
    [86] = { 65,  45,  55,  45,  70, TYPE_WATER,    TYPE_WATER,  190, 100 }, // Seel
    [87] = { 90,  70,  80,  70,  95, TYPE_WATER,    TYPE_ICE,     75, 176 }, // Dewgong

    // ── Grimer line ───────────────────────────────────────────────────────────
    [88] = { 80,  80,  50,  25,  40, TYPE_POISON,   TYPE_POISON, 190,  90 }, // Grimer
    [89] = {105, 105,  75,  50,  65, TYPE_POISON,   TYPE_POISON,  75, 157 }, // Muk

    // ── Shellder line ─────────────────────────────────────────────────────────
    [90] = { 30,  65, 100,  40,  45, TYPE_WATER,    TYPE_WATER,  190,  97 }, // Shellder
    [91] = { 50,  95, 180,  70,  85, TYPE_WATER,    TYPE_ICE,     60, 203 }, // Cloyster

    // ── Gastly line ───────────────────────────────────────────────────────────
    [92] = { 30,  35,  30,  80, 100, TYPE_GHOST,    TYPE_POISON, 190,  95 }, // Gastly
    [93] = { 45,  50,  45,  95, 115, TYPE_GHOST,    TYPE_POISON,  90, 126 }, // Haunter
    [94] = { 60,  65,  60, 110, 130, TYPE_GHOST,    TYPE_POISON,  45, 190 }, // Gengar

    // ── Onix ──────────────────────────────────────────────────────────────────
    [95] = { 35,  45, 160,  70,  30, TYPE_ROCK,     TYPE_GROUND,  45, 108 }, // Onix

    // ── Drowzee line ──────────────────────────────────────────────────────────
    [96] = { 60,  48,  45,  42,  90, TYPE_PSYCHIC,  TYPE_PSYCHIC, 190,102 }, // Drowzee
    [97] = { 85,  73,  70,  67, 115, TYPE_PSYCHIC,  TYPE_PSYCHIC,  75,165 }, // Hypno

    // ── Krabby line ───────────────────────────────────────────────────────────
    [98] = { 30, 105,  90,  50,  25, TYPE_WATER,    TYPE_WATER,  225, 115 }, // Krabby
    [99] = { 55, 130, 115,  75,  50, TYPE_WATER,    TYPE_WATER,   60, 206 }, // Kingler

    // ── Voltorb line ──────────────────────────────────────────────────────────
    [100] = { 40,  30,  50, 100,  55, TYPE_ELECTRIC, TYPE_ELECTRIC,190,103 }, // Voltorb
    [101] = { 60,  50,  70, 140,  80, TYPE_ELECTRIC, TYPE_ELECTRIC, 60,150 }, // Electrode

    // ── Exeggcute line ────────────────────────────────────────────────────────
    [102] = { 60,  40,  80,  40,  60, TYPE_GRASS,    TYPE_PSYCHIC,  90, 98 }, // Exeggcute
    [103] = { 95,  95,  85,  55, 125, TYPE_GRASS,    TYPE_PSYCHIC,  45,212 }, // Exeggutor

    // ── Cubone line ───────────────────────────────────────────────────────────
    [104] = { 50,  50,  95,  35,  40, TYPE_GROUND,   TYPE_GROUND, 190,  87 }, // Cubone
    [105] = { 60,  80, 110,  45,  50, TYPE_GROUND,   TYPE_GROUND,  75, 124 }, // Marowak

    // ── Hitmons ───────────────────────────────────────────────────────────────
    [106] = { 50, 120,  53,  87,  35, TYPE_FIGHTING, TYPE_FIGHTING, 45,139 }, // Hitmonlee
    [107] = { 50, 105,  79,  76,  35, TYPE_FIGHTING, TYPE_FIGHTING, 45,140 }, // Hitmonchan

    // ── Lickitung ─────────────────────────────────────────────────────────────
    [108] = { 90,  55,  75,  30,  60, TYPE_NORMAL,   TYPE_NORMAL,  45, 127 }, // Lickitung

    // ── Koffing line ──────────────────────────────────────────────────────────
    [109] = { 40,  65,  95,  35,  60, TYPE_POISON,   TYPE_POISON, 190, 114 }, // Koffing
    [110] = { 65,  90, 120,  60,  85, TYPE_POISON,   TYPE_POISON,  60, 173 }, // Weezing

    // ── Rhyhorn line ──────────────────────────────────────────────────────────
    [111] = { 80,  85,  95,  25,  30, TYPE_GROUND,   TYPE_ROCK,   120, 135 }, // Rhyhorn
    [112] = {105, 130, 120,  40,  45, TYPE_GROUND,   TYPE_ROCK,    60, 204 }, // Rhydon

    // ── Chansey ───────────────────────────────────────────────────────────────
    [113] = {250,   5,   5,  50, 105, TYPE_NORMAL,   TYPE_NORMAL,  30, 255 }, // Chansey

    // ── Tangela ───────────────────────────────────────────────────────────────
    [114] = { 65,  55, 115,  60, 100, TYPE_GRASS,    TYPE_GRASS,   45, 166 }, // Tangela

    // ── Kangaskhan ────────────────────────────────────────────────────────────
    [115] = {105,  95,  80,  90,  40, TYPE_NORMAL,   TYPE_NORMAL,  45, 175 }, // Kangaskhan

    // ── Horsea line ───────────────────────────────────────────────────────────
    [116] = { 30,  40,  70,  60,  70, TYPE_WATER,    TYPE_WATER,  225,  83 }, // Horsea
    [117] = { 55,  65,  95,  85,  95, TYPE_WATER,    TYPE_WATER,   75, 155 }, // Seadra

    // ── Goldeen line ──────────────────────────────────────────────────────────
    [118] = { 45,  67,  60,  63,  50, TYPE_WATER,    TYPE_WATER,  225, 111 }, // Goldeen
    [119] = { 80,  92,  65,  68,  80, TYPE_WATER,    TYPE_WATER,   60, 170 }, // Seaking

    // ── Staryu line ───────────────────────────────────────────────────────────
    [120] = { 30,  45,  55,  85,  70, TYPE_WATER,    TYPE_WATER,  225, 106 }, // Staryu
    [121] = { 60,  75,  85, 115, 100, TYPE_WATER,    TYPE_PSYCHIC,  60,207 }, // Starmie

    // ── Mr. Mime ──────────────────────────────────────────────────────────────
    [122] = { 40,  45,  65,  90, 100, TYPE_PSYCHIC,  TYPE_PSYCHIC,  45,136 }, // Mr. Mime

    // ── Scyther ───────────────────────────────────────────────────────────────
    [123] = { 70, 110,  80, 105,  55, TYPE_BUG,      TYPE_FLYING,  45, 187 }, // Scyther

    // ── Jynx ──────────────────────────────────────────────────────────────────
    [124] = { 65,  50,  35,  95,  95, TYPE_ICE,      TYPE_PSYCHIC,  45,137 }, // Jynx

    // ── Electabuzz ────────────────────────────────────────────────────────────
    [125] = { 65,  83,  57, 105,  85, TYPE_ELECTRIC, TYPE_ELECTRIC, 45,156 }, // Electabuzz

    // ── Magmar ────────────────────────────────────────────────────────────────
    [126] = { 65,  95,  57,  93,  85, TYPE_FIRE,     TYPE_FIRE,    45, 167 }, // Magmar

    // ── Pinsir ────────────────────────────────────────────────────────────────
    [127] = { 65, 125, 100,  85,  55, TYPE_BUG,      TYPE_BUG,     45, 200 }, // Pinsir

    // ── Tauros ────────────────────────────────────────────────────────────────
    [128] = { 75, 100,  95, 110,  70, TYPE_NORMAL,   TYPE_NORMAL,  45, 211 }, // Tauros

    // ── Magikarp line ─────────────────────────────────────────────────────────
    [129] = { 20,  10,  55,  80,  20, TYPE_WATER,    TYPE_WATER,  255,  20 }, // Magikarp
    [130] = { 95, 125,  79,  81, 100, TYPE_WATER,    TYPE_FLYING,  45, 214 }, // Gyarados

    // ── Lapras ────────────────────────────────────────────────────────────────
    [131] = {130,  85,  80,  60,  95, TYPE_WATER,    TYPE_ICE,     45, 219 }, // Lapras

    // ── Ditto ─────────────────────────────────────────────────────────────────
    [132] = { 48,  48,  48,  48,  48, TYPE_NORMAL,   TYPE_NORMAL,  35,  61 }, // Ditto

    // ── Eevee evolutions ──────────────────────────────────────────────────────
    [133] = { 55,  55,  50,  55,  65, TYPE_NORMAL,   TYPE_NORMAL,  45,  92 }, // Eevee
    [134] = {130,  65,  60,  65, 110, TYPE_WATER,    TYPE_WATER,   45, 196 }, // Vaporeon
    [135] = { 65,  65,  60, 130, 110, TYPE_ELECTRIC, TYPE_ELECTRIC, 45,197 }, // Jolteon
    [136] = { 65, 130,  60,  65, 110, TYPE_FIRE,     TYPE_FIRE,    45, 198 }, // Flareon

    // ── Porygon ───────────────────────────────────────────────────────────────
    [137] = { 65,  60,  70,  40,  75, TYPE_NORMAL,   TYPE_NORMAL,  45, 130 }, // Porygon

    // ── Fossil Pokémon ────────────────────────────────────────────────────────
    [138] = { 35,  40, 100,  35,  90, TYPE_ROCK,     TYPE_WATER,   45, 120 }, // Omanyte
    [139] = { 70,  60, 125,  55, 115, TYPE_ROCK,     TYPE_WATER,   45, 199 }, // Omastar
    [140] = { 30,  80,  90,  55,  45, TYPE_ROCK,     TYPE_WATER,   45, 119 }, // Kabuto
    [141] = { 60, 115, 105,  80,  70, TYPE_ROCK,     TYPE_WATER,   45, 201 }, // Kabutops
    [142] = { 80, 105,  65, 130,  60, TYPE_ROCK,     TYPE_FLYING,  45, 202 }, // Aerodactyl

    // ── Snorlax ───────────────────────────────────────────────────────────────
    [143] = {160, 110,  65,  30,  65, TYPE_NORMAL,   TYPE_NORMAL,  25, 154 }, // Snorlax

    // ── Legendary birds ───────────────────────────────────────────────────────
    [144] = { 90,  85, 100,  85, 125, TYPE_ICE,      TYPE_FLYING,   3, 215 }, // Articuno
    [145] = { 90,  90,  85, 100, 125, TYPE_ELECTRIC, TYPE_FLYING,   3, 216 }, // Zapdos
    [146] = { 90, 100,  90,  90, 125, TYPE_FIRE,     TYPE_FLYING,   3, 217 }, // Moltres

    // ── Dratini line ──────────────────────────────────────────────────────────
    [147] = { 41,  64,  45,  50,  50, TYPE_DRAGON,   TYPE_DRAGON,  45,  67 }, // Dratini
    [148] = { 61,  84,  65,  70,  70, TYPE_DRAGON,   TYPE_DRAGON,  45, 144 }, // Dragonair
    [149] = { 91, 134,  95,  80, 100, TYPE_DRAGON,   TYPE_FLYING,  45, 218 }, // Dragonite

    // ── Mewtwo ────────────────────────────────────────────────────────────────
    [150] = {106, 110,  90, 130, 154, TYPE_PSYCHIC,  TYPE_PSYCHIC,   3, 220 }, // Mewtwo

    // ── Mew ───────────────────────────────────────────────────────────────────
    [151] = {100, 100, 100, 100, 100, TYPE_NORMAL,   TYPE_NORMAL,  45, 64  }, // Mew
};
