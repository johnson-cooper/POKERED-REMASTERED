#pragma once
#include "types.h"

// Top-level game state machine
typedef enum {
    GAME_STATE_BOOT = 0,
    GAME_STATE_TITLE,
    GAME_STATE_INTRO,
    GAME_STATE_OVERWORLD,
    GAME_STATE_BATTLE,
    GAME_STATE_MENU,
} GameState;

typedef struct {
    GameState state;
    GameState next_state;
    u32       frame;
} GameContext;

extern GameContext g_game;
extern u32 g_player_money;

#define STARTING_MONEY 3000

void game_init(void);
void game_update(void);
void game_change_state(GameState new_state);
const char *game_get_player_name(void);
const char *game_get_rival_name(void);
void game_add_money(u32 amount);
void game_subtract_money(u32 amount);

// Pokémon nickname screen used by starter selection.
void game_nickname_open(const char *default_name);
bool8 game_nickname_active(void);
bool8 game_nickname_update(char *out_name, u8 out_size);
const char *game_get_nickname_result(void);
