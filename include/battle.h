#pragma once
#include "pokemon.h"
#include "types.h"
#include "types.h"

void battle_setup_rival(u16 chosen_ball, const char *player_nickname);
void battle_setup_wild(PokemonId species, u8 level, PokemonId player_species,
                       const char *player_nickname);
bool8 battle_is_wild(void);
bool8 battle_is_blackout(void);
void battle_init(void);
void battle_transition_start(void);
void battle_update(void);
