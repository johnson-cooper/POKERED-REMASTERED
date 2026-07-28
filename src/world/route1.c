#include "route1.h"
#include "world.h"
#include "battle.h"
#include "battle_rng.h"
#include "game.h"
#include "party.h"
#include "map_ids.h"
#include "wild_encounters.h"

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
    PokemonId species = MON_NONE;
    wild_encounter_select(MAP_ROUTE_1, &species, level);
    return species;
}

bool8 route1_try_wild_encounter(void) {
    if (!route1_player_in_grass()) return FALSE;

    u8 level;
    PokemonId species = MON_NONE;
    if (!wild_encounter_select(MAP_ROUTE_1, &species, &level)) return FALSE;
    PartyPokemon *active = party_get_lead();
    PokemonId player_species = active ? active->species : MON_BULBASAUR;
    const char *player_nickname = active ? (active->nickname[0] ? active->nickname : NULL) : NULL;
    battle_setup_wild(species, level, player_species, player_nickname);
    game_change_state(GAME_STATE_BATTLE);
    return TRUE;
}
