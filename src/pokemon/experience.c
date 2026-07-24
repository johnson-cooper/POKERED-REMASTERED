#include "experience.h"

// Growth-rate assignments in Pokédex order, copied from the local pokered
// reference's base_stats files. Index 0 is the unused MON_NONE slot.
static const char s_growth_rates[] =
    "0SSSSSSSSSFFFFFFSSSFFFFFFFFFFSSSSSSAAFFAAFFSSSFFFFFFFFFFFFLLSSSSSSSSSSSSLLSSSFFFFFFFFFFFFFLLSSSFFFFFFFLLFFFFFFFLLAFFFFFFLLFFFFFLLLLLFFFFFFFFFFLLLLLLLLLS";

u32 pokemon_exp_for_level(PokemonId species, u8 level) {
    if (species > NUM_POKEMON) species = MON_NONE;
    if (level > 100) level = 100;

    u32 n = level;
    u32 square = n * n;
    u32 cube = square * n;
    switch (s_growth_rates[species]) {
    case 'F': return cube;                       // Medium Fast
    case 'A': return (4 * cube) / 5;             // Fast
    case 'L': return (5 * cube) / 4;             // Slow
    case 'S': return (6 * cube) / 5 - 15 * square + 100 * n - 140;
    default:  return cube;
    }
}
