#include "viridian_forest.h"
#include "world.h"
#include "battle.h"
#include "battle_rng.h"
#include "game.h"
#include "party.h"
#include "map_ids.h"
#include "wild_encounters.h"

static bool8 forest_player_in_grass(void) {
    if (!g_world.map || g_world.map->map_id != MAP_VIRIDIAN_FOREST)
        return FALSE;
    // Pokered's Forest tileset header declares only $20 as the encounter
    // grass tile. $30 is a common filled/background tile in the blockset and
    // must not be treated as encounter grass.
    u16 tile = map_get_subtile_tile_id(g_world.player.tile_x,
                                       g_world.player.tile_y);
    return tile == 0x20;
}

bool8 viridian_forest_try_wild_encounter(void) {
    if (!forest_player_in_grass()) return FALSE;

    u8 level;
    PokemonId species = MON_NONE;
    if (!wild_encounter_select(MAP_VIRIDIAN_FOREST, &species, &level))
        return FALSE;
    PartyPokemon *active = party_get_lead();
    PokemonId player_species = active ? active->species : MON_BULBASAUR;
    const char *nickname = active && active->nickname[0] ? active->nickname : NULL;
    battle_setup_wild(species, level, player_species, nickname);
    game_change_state(GAME_STATE_BATTLE);
    return TRUE;
}
