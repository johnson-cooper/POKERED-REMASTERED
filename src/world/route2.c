#include "route2.h"
#include "world.h"
#include "battle.h"
#include "battle_rng.h"
#include "game.h"
#include "party.h"
#include "map_ids.h"
#include "wild_encounters.h"

static bool8 route2_player_in_grass(void) {
    if (!g_world.map || g_world.map->map_id != MAP_ROUTE_2)
        return FALSE;
    MapCell cell = map_get_cell(g_world.player.tile_x / 2,
                                g_world.player.tile_y / 2);
    return MAPCELL_METATILE(cell) == 0x0B;
}

bool8 route2_try_wild_encounter(void) {
    if (!route2_player_in_grass()) return FALSE;

    u8 level;
    PokemonId mon = MON_NONE;
    if (!wild_encounter_select(MAP_ROUTE_2, &mon, &level)) return FALSE;
    PartyPokemon *active = party_get_lead();
    PokemonId player_species = active ? active->species : MON_BULBASAUR;
    const char *player_nickname = active ? (active->nickname[0] ? active->nickname : NULL) : NULL;
    battle_setup_wild(mon, level, player_species, player_nickname);
    game_change_state(GAME_STATE_BATTLE);
    return TRUE;
}
