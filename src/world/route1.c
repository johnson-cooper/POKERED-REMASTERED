#include "route1.h"
#include "world.h"
#include "battle.h"
#include "battle_rng.h"
#include "game.h"
#include "party.h"
#include "map_ids.h"

static bool8 route1_player_in_grass(void) {
    if (!g_world.map || g_world.map->map_id != MAP_ROUTE_1)
        return FALSE;
    MapCell cell = map_get_cell(g_world.player.tile_x / 2,
                                g_world.player.tile_y / 2);
    // Route 1's reference grass block is block $0B, whose tiles are the
    // overworld grass tile $52.
    return MAPCELL_METATILE(cell) == 0x0B;
}

PokemonId route1_random_species(u8 *level) {
    static const PokemonId species[10] = {
        MON_PIDGEY, MON_RATTATA, MON_RATTATA, MON_RATTATA,
        MON_PIDGEY, MON_PIDGEY, MON_PIDGEY, MON_RATTATA,
        MON_PIDGEY, MON_PIDGEY,
    };
    static const u8 levels[10] = { 3, 3, 3, 2, 2, 3, 3, 4, 4, 5 };
    static const u8 cumulative[10] = { 50, 101, 140, 165, 190,
                                       215, 228, 241, 252, 255 };
    u8 roll = (u8)(battle_random() & 0xFF);
    u8 slot = 0;
    while (slot < 9 && roll > cumulative[slot]) slot++;
    *level = levels[slot];
    return species[slot];
}

bool8 route1_try_wild_encounter(void) {
    if (!route1_player_in_grass()) return FALSE;

    // Reference Route1 encounter rate: 25/256 per completed step.
    if ((battle_random() & 0xFF) >= 25) return FALSE;

    u8 level;
    PokemonId species = route1_random_species(&level);
    PartyPokemon *active = party_get_active();
    PokemonId player_species = active ? active->species : MON_BULBASAUR;
    const char *player_nickname = active ? (active->nickname[0] ? active->nickname : NULL) : NULL;
    battle_setup_wild(species, level, player_species, player_nickname);
    game_change_state(GAME_STATE_BATTLE);
    return TRUE;
}
