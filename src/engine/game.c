#include "game.h"
#include "render.h"
#include "input.h"
#include "world.h"
#include "text.h"
#include "dialog.h"
#include "flags.h"
#include "map_ids.h"
#include "title.h"
#include "battle.h"

GameContext g_game = {
    .state      = GAME_STATE_BOOT,
    .next_state = GAME_STATE_BOOT,
    .frame      = 0,
};

extern const MapHeader g_map_reds_house_2f;

static void state_boot_update(void);
static void state_title_update(void);
static void state_intro_update(void);
static void state_overworld_update(void);
static void state_battle_update(void);
static void state_menu_update(void);

typedef enum {
    INTRO_OAK_DIALOG = 0,
    INTRO_NIDORINO_DIALOG,
    INTRO_PLAYER_DIALOG,
    INTRO_PLAYER_NAME_SELECT,
    INTRO_PLAYER_NAME_CONFIRM,
    INTRO_RIVAL_INTRO,
    INTRO_RIVAL_NAME_SELECT,
    INTRO_RIVAL_NAME_CONFIRM,
    INTRO_DONE,
} IntroState;

static IntroState s_intro_state = INTRO_OAK_DIALOG;
static char s_player_name[8];
static char s_rival_name[8];
static char s_intro_dialog[128];
static bool8 s_selecting_rival;
static u8 s_name_row;
static u8 s_name_col;
static bool8 s_nickname_active;
static char s_nickname[8];
static u8 s_nickname_row;
static u8 s_nickname_col;

const char *game_get_player_name(void) {
    return s_player_name;
}

const char *game_get_rival_name(void) {
    return s_rival_name;
}

// The original pokered naming screen is a 9-column character grid followed by
// delete/end controls. Keep the same basic flow while the rest of the intro is
// still being built.
static const char s_name_grid[6][10] = {
    "ABCDEFGHI",
    "JKLMNOPQR",
    "STUVWXYZ'",
    "abcdefghi",
    "jklmnopqr",
    "stuvwxyz-",
};

static void intro_name_screen_draw(void);
static void intro_name_screen_update(void);
static void intro_dialog_open(const char *prefix, const char *name, const char *suffix);
static void nickname_screen_draw(void);

typedef void (*StateUpdateFn)(void);
static const StateUpdateFn STATE_UPDATE_TABLE[] = {
    [GAME_STATE_BOOT]       = state_boot_update,
    [GAME_STATE_TITLE]      = state_title_update,
    [GAME_STATE_INTRO]      = state_intro_update,
    [GAME_STATE_OVERWORLD]  = state_overworld_update,
    [GAME_STATE_BATTLE]     = state_battle_update,
    [GAME_STATE_MENU]       = state_menu_update,
};

void game_init(void) {
    g_game.state      = GAME_STATE_BOOT;
    g_game.next_state = GAME_STATE_BOOT;
    g_game.frame      = 0;
}

void game_update(void) {
    input_update();

    if (g_game.next_state != g_game.state) {
        GameState previous_state = g_game.state;
        GameState entering = g_game.next_state;
        g_game.state = entering;

        if (entering == GAME_STATE_TITLE) {
            title_draw();
            text_clear();
            text_draw_str_pal(8, 9, "RED REMASTERED", 13);
            text_draw_str(9, 18, "PRESS START");
        } else if (entering == GAME_STATE_INTRO) {
            intro_graphics_show(INTRO_GFX_OAK);
            text_clear();
            s_intro_state = INTRO_OAK_DIALOG;
            s_player_name[0] = '\0';
            s_rival_name[0] = '\0';
            s_selecting_rival = FALSE;
            s_name_row = 0;
            s_name_col = 0;
            dialog_open();
            dialog_set_text(
                "Hello there!\f"
                "Welcome to the world of\nPOK~MON!\f"
                "My name is OAK! People call\nme the POK~MON PROF!");
        } else if (entering == GAME_STATE_OVERWORLD) {
            // A fresh game uses the temporary Red's House 2F spawn.  Returning
            // from battle must keep the current map (Oak's Lab) intact.
            if (previous_state != GAME_STATE_BATTLE) {
                flags_clear_all();
                // Spawn in Red's House 2F, center of room.
                world_init(&g_map_reds_house_2f, 3, 3);
            }
        } else if (entering == GAME_STATE_BATTLE) {
            battle_init();
        }
    }

    if (g_game.state < ARRAY_COUNT(STATE_UPDATE_TABLE) &&
        STATE_UPDATE_TABLE[g_game.state] != NULL) {
        STATE_UPDATE_TABLE[g_game.state]();
    }

    g_game.frame++;
}

void game_change_state(GameState new_state) {
    g_game.next_state = new_state;
}

static void state_boot_update(void) {
    game_change_state(GAME_STATE_TITLE);
}

static void state_title_update(void) {
    // Blink "Press START"
    if ((g_game.frame & 30) < 20) {
        text_draw_str(9, 18, "PRESS START");
    } else {
        text_draw_str(9, 18, "           ");
    }
    if (input_pressed(KEY_START))
        game_change_state(GAME_STATE_INTRO);
}

static void state_intro_update(void) {
    switch (s_intro_state) {
    case INTRO_OAK_DIALOG:
        if (dialog_update()) {
            intro_graphics_show(INTRO_GFX_NIDORINO);
            dialog_open();
            dialog_set_text(
                "This world is inhabited by\ncreatures called POK~MON!\f"
                "For some people, POK~MON are\npets. Others use them for\f"
                "fights. Myself... I study\nPOK~MON as a profession.");
            s_intro_state = INTRO_NIDORINO_DIALOG;
        }
        break;
    case INTRO_NIDORINO_DIALOG:
        if (dialog_update()) {
            intro_graphics_show(INTRO_GFX_RED);
            dialog_open();
            dialog_set_text("First, what is your name?");
            s_selecting_rival = FALSE;
            s_intro_state = INTRO_PLAYER_DIALOG;
        }
        break;
    case INTRO_PLAYER_DIALOG:
        if (dialog_update()) {
            intro_name_screen_draw();
            s_intro_state = INTRO_PLAYER_NAME_SELECT;
        }
        break;
    case INTRO_PLAYER_NAME_SELECT:
        intro_name_screen_update();
        break;
    case INTRO_PLAYER_NAME_CONFIRM:
        if (dialog_update()) {
            intro_graphics_show(INTRO_GFX_RIVAL);
            dialog_open();
            dialog_set_text(
                "This is my grandson. He's\nbeen your rival since you\f"
                "were a baby.\f"
                "...Erm, what is his name\nagain?");
            s_intro_state = INTRO_RIVAL_INTRO;
        }
        break;
    case INTRO_RIVAL_INTRO:
        if (dialog_update()) {
            s_selecting_rival = TRUE;
            s_intro_state = INTRO_RIVAL_NAME_SELECT;
            intro_name_screen_draw();
        }
        break;
    case INTRO_RIVAL_NAME_SELECT:
        intro_name_screen_update();
        break;
    case INTRO_RIVAL_NAME_CONFIRM:
        if (dialog_update()) {
            intro_graphics_show(INTRO_GFX_RED);
            dialog_open();
            intro_dialog_open("", s_player_name,
                "!\fYour very own POK~MON legend\nis about to unfold!\f"
                "A world of dreams and\nadventures with POK~MON\f"
                "awaits! Let's go!");
            s_intro_state = INTRO_DONE;
        }
        break;
    case INTRO_DONE:
        if (dialog_update())
            game_change_state(GAME_STATE_OVERWORLD);
        break;
    }
}

static void intro_name_screen_draw(void) {
    const char *name = s_selecting_rival ? s_rival_name : s_player_name;

    title_hide();
    text_clear();
    text_draw_str(9, 1, s_selecting_rival ? "RIVAL NAME" : "NAME SELECT");
    text_draw_str(3, 3, s_selecting_rival ? "RIVAL:" : "NAME:");
    text_draw_str(10, 3, name);

    for (u8 row = 0; row < 6; row++) {
        text_draw_str(4, (u8)(6 + row), s_name_grid[row]);
        text_draw_char(3, (u8)(6 + row), ' ');
    }

    text_draw_str(4, 13, "DEL");
    text_draw_str(15, 13, "END");

    if (s_name_row < 6)
        text_draw_char((u8)(4 + s_name_col), (u8)(6 + s_name_row), '>');
    else if (s_name_col == 0)
        text_draw_char(3, 13, '>');
    else
        text_draw_char(14, 13, '>');

    text_draw_str(3, 15, "A:SELECT  B:DELETE");
    text_draw_str(5, 16, "START:END");
}

static void intro_name_screen_update(void) {
    char *name = s_selecting_rival ? s_rival_name : s_player_name;
    bool8 moved = FALSE;

    if (input_pressed(KEY_UP)) {
        if (s_name_row == 0) s_name_row = 6;
        else s_name_row--;
        moved = TRUE;
    } else if (input_pressed(KEY_DOWN)) {
        if (s_name_row >= 6) s_name_row = 0;
        else s_name_row++;
        moved = TRUE;
    } else if (input_pressed(KEY_LEFT)) {
        if (s_name_row < 6) {
            if (s_name_col == 0) s_name_col = 8;
            else s_name_col--;
        } else {
            s_name_col = 0;
        }
        moved = TRUE;
    } else if (input_pressed(KEY_RIGHT)) {
        if (s_name_row < 6) {
            if (s_name_col >= 8) s_name_col = 0;
            else s_name_col++;
        } else {
            s_name_col = 1;
        }
        moved = TRUE;
    }

    if (input_pressed(KEY_A)) {
        if (s_name_row < 6) {
            u8 len = 0;
            while (name[len] != '\0' && len < 7) len++;
            if (len < 7) {
                name[len] = s_name_grid[s_name_row][s_name_col];
                name[len + 1] = '\0';
            }
        } else if (s_name_col == 0) {
            u8 len = 0;
            while (name[len] != '\0' && len < 7) len++;
            if (len > 0) name[len - 1] = '\0';
        } else if (name[0] != '\0') {
            text_clear();
            dialog_open();
            if (s_selecting_rival) {
                intro_dialog_open("That's right! I remember\nnow! His name is ", name, "!");
                s_intro_state = INTRO_RIVAL_NAME_CONFIRM;
            } else {
                intro_dialog_open("Right! So your\nname is ", name, "!");
                s_intro_state = INTRO_PLAYER_NAME_CONFIRM;
            }
            return;
        }
        moved = TRUE;
    }

    if (input_pressed(KEY_B)) {
        u8 len = 0;
        while (name[len] != '\0' && len < 7) len++;
        if (len > 0) name[len - 1] = '\0';
        moved = TRUE;
    }

    if (input_pressed(KEY_START) && name[0] != '\0') {
        text_clear();
        dialog_open();
        if (s_selecting_rival) {
            intro_dialog_open("That's right! I remember\nnow! His name is ", name, "!");
            s_intro_state = INTRO_RIVAL_NAME_CONFIRM;
        } else {
            intro_dialog_open("Right! So your\nname is ", name, "!");
            s_intro_state = INTRO_PLAYER_NAME_CONFIRM;
        }
        return;
    }

    if (moved)
        intro_name_screen_draw();
}

void game_nickname_open(const char *default_name) {
    u8 i = 0;
    while (default_name[i] && i < 7) {
        s_nickname[i] = default_name[i];
        i++;
    }
    s_nickname[i] = '\0';
    s_nickname_row = 0;
    s_nickname_col = 0;
    s_nickname_active = TRUE;
    nickname_screen_draw();
}

bool8 game_nickname_active(void) {
    return s_nickname_active;
}

const char *game_get_nickname_result(void) {
    return s_nickname;
}

bool8 game_nickname_update(char *out_name, u8 out_size) {
    bool8 moved = FALSE;

    if (input_pressed(KEY_UP)) {
        if (s_nickname_row == 0) s_nickname_row = 6;
        else s_nickname_row--;
        moved = TRUE;
    } else if (input_pressed(KEY_DOWN)) {
        if (s_nickname_row >= 6) s_nickname_row = 0;
        else s_nickname_row++;
        moved = TRUE;
    } else if (input_pressed(KEY_LEFT)) {
        if (s_nickname_row < 6) {
            if (s_nickname_col == 0) s_nickname_col = 8;
            else s_nickname_col--;
        } else {
            s_nickname_col = 0;
        }
        moved = TRUE;
    } else if (input_pressed(KEY_RIGHT)) {
        if (s_nickname_row < 6) {
            if (s_nickname_col >= 8) s_nickname_col = 0;
            else s_nickname_col++;
        } else {
            s_nickname_col = 1;
        }
        moved = TRUE;
    }

    if (input_pressed(KEY_B)) {
        u8 len = 0;
        while (s_nickname[len] && len < 7) len++;
        if (len) s_nickname[len - 1] = '\0';
        moved = TRUE;
    }

    if (input_pressed(KEY_A)) {
        if (s_nickname_row < 6) {
            u8 len = 0;
            while (s_nickname[len] && len < 7) len++;
            if (len < 7) {
                s_nickname[len] = s_name_grid[s_nickname_row][s_nickname_col];
                s_nickname[len + 1] = '\0';
            }
        } else if (s_nickname_col == 0) {
            u8 len = 0;
            while (s_nickname[len] && len < 7) len++;
            if (len) s_nickname[len - 1] = '\0';
        } else if (s_nickname[0]) {
            for (u8 i = 0; i + 1 < out_size; i++)
                out_name[i] = s_nickname[i];
            out_name[out_size - 1] = '\0';
            s_nickname_active = FALSE;
            text_clear();
            return TRUE;
        }
        moved = TRUE;
    }

    if (moved) nickname_screen_draw();
    return FALSE;
}

static void nickname_screen_draw(void) {
    // Naming is a full-screen mode in pokered; make every visible BG0 tile
    // opaque before drawing the controls so the overworld cannot show through.
    text_fill_opaque();
    text_draw_str(8, 1, "NAME POKEMON");
    text_draw_str(3, 3, "NAME:");
    text_draw_str(10, 3, s_nickname);

    for (u8 row = 0; row < 6; row++) {
        text_draw_str(4, (u8)(6 + row), s_name_grid[row]);
        text_draw_char(3, (u8)(6 + row), ' ');
    }
    text_draw_str(4, 13, "DEL");
    text_draw_str(15, 13, "END");

    if (s_nickname_row < 6)
        text_draw_char((u8)(4 + s_nickname_col), (u8)(6 + s_nickname_row), '>');
    else if (s_nickname_col == 0)
        text_draw_char(3, 13, '>');
    else
        text_draw_char(14, 13, '>');

    text_draw_str(3, 15, "A:SELECT  B:DELETE");
    text_draw_str(5, 16, "START:END");
}

static void intro_dialog_open(const char *prefix, const char *name, const char *suffix) {
    u8 i = 0;

    while (*prefix && i < sizeof(s_intro_dialog) - 1)
        s_intro_dialog[i++] = *prefix++;
    while (*name && i < sizeof(s_intro_dialog) - 1)
        s_intro_dialog[i++] = *name++;
    while (*suffix && i < sizeof(s_intro_dialog) - 1)
        s_intro_dialog[i++] = *suffix++;
    s_intro_dialog[i] = '\0';
    dialog_set_text(s_intro_dialog);
}

static void state_overworld_update(void) {
    if (s_nickname_active) {
        char result[8];
        if (game_nickname_update(result, sizeof(result))) {
            // The script consumes the completed name on its next tick.
        }
        return;
    }
    world_update();
    world_render();
}

static void state_battle_update(void) {
    battle_update();
}

static void state_menu_update(void) {
    // Phase 5 — not yet implemented
}
