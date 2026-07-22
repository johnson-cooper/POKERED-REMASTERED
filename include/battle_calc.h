#pragma once
#include "types.h"
#include "battle_pokemon.h"

u16  battle_calc_damage(const BattlePokemon *atk, const BattlePokemon *def,
                        MoveId move, bool8 is_critical);
bool8 battle_check_critical(const BattlePokemon *atk, MoveId move);
bool8 battle_check_hit(const BattlePokemon *atk, const BattlePokemon *def, MoveId move);
bool8 battle_move_is_physical(PokemonType type);
