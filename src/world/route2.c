#include "route2.h"
#include "world.h"
#include "battle.h"
#include "battle_rng.h"
#include "game.h"
#include "party.h"
#include "map_ids.h"

static bool8 route2_player_in_grass(void) {
    if (!g_world.map || g_world.map->map_id != MAP_ROUTE_2)
        return FALSE;
    MapCell cell = map_get_cell(g_world.player.tile_x / 2,
                                g_world.player.tile_y / 2);
    return MAPCELL_METATILE(cell) == 0x0B;
}

bool8 route2_try_wild_encounter(void) {
    if (!route2_player_in_grass()) return FALSE;

    if ((battle_random() & 0xFF) >= 25) return FALSE;

    // pokered Red Route 2 encounter table (10 slots):
    // Lv3 Rattata, Lv3 Pidgey, Lv4 Pidgey, Lv4 Rattata,
    // Lv5 Pidgey, Lv3 Weedle, Lv2 Rattata, Lv5 Rattata,
    // Lv4 Weedle, Lv5 Weedle
    static const PokemonId species[10] = {
        MON_RATTATA, MON_PIDGEY, MON_PIDGEY, MON_RATTATA,
        MON_PIDGEY, MON_WEEDLE, MON_RATTATA, MON_RATTATA,
        MON_WEEDLE, MON_WEEDLE,
    };
    static const u8 levels[10] = { 3, 3, 4, 4, 5, 3, 2, 5, 4, 5 };
    static const u8 cumulative[10] = { 50, 101, 140, 165, 190,
                                       215, 228, 241, 252, 255 };

    u8 roll = (u8)(battle_random() & 0xFF);
    u8 slot = 0;
    while (slot < 9 && roll > cumulative[slot]) slot++;

    u8 level = levels[slot];
    PokemonId mon = species[slot];
    PartyPokemon *active = party_get_lead();
    PokemonId player_species = active ? active->species : MON_BULBASAUR;
    const char *player_nickname = active ? (active->nickname[0] ? active->nickname : NULL) : NULL;
    battle_setup_wild(mon, level, player_species, player_nickname);
    game_change_state(GAME_STATE_BATTLE);
    return TRUE;
}
