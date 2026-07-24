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
#include "pokedex.h"
#include "save.h"
#include "script.h"
#include "audio.h"

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
static void title_menu_draw(void);
static void title_menu_update(void);
static void title_options_draw(void);
static void title_options_update(void);
static void title_draw_menu_box(u8 left, u8 top, u8 right, u8 bottom);
static void pause_menu_draw(void);
static void pause_menu_update(void);

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

typedef enum {
    TITLE_MODE_PRESS_START = 0,
    TITLE_MODE_MAIN_MENU,
    TITLE_MODE_OPTIONS,
} TitleMode;

static TitleMode s_title_mode = TITLE_MODE_PRESS_START;
static u8 s_title_menu_cursor;
static u8 s_title_option_cursor;
static bool8 s_title_has_save;
static bool8 s_option_fast_text;
static bool8 s_option_battle_animation = TRUE;
static bool8 s_option_battle_style;
static bool8 s_options_from_pause;
static u8 s_pause_menu_cursor;
static bool8 s_pause_pokedex_active;
static bool8 s_continue_load;

static bool8 title_save_available(void) {
    return save_exists();
}

static void copy_name(char dst[SAVE_NAME_LENGTH], const char *src) {
    for (u8 i = 0; i < SAVE_NAME_LENGTH; i++) {
        dst[i] = src[i];
        if (!src[i]) break;
    }
    dst[SAVE_NAME_LENGTH - 1] = '\0';
}

static void save_capture(SaveData *data) {
    for (u32 i = 0; i < sizeof(SaveData); i++) ((u8 *)data)[i] = 0;
    data->map_id = g_world.map ? g_world.map->map_id : MAP_PLAYERS_HOUSE_2F;
    data->last_map_id = g_world.last_map ? g_world.last_map->map_id : data->map_id;
    data->player_x = (u8)g_world.player.tile_x;
    data->player_y = (u8)g_world.player.tile_y;
    data->player_facing = (u8)g_world.player.facing;
    copy_name((char *)data->player_name, s_player_name);
    copy_name((char *)data->rival_name, s_rival_name);
    flags_export(data->flags);
    data->option_fast_text = s_option_fast_text;
    data->option_battle_animation = s_option_battle_animation;
    data->option_battle_style = s_option_battle_style;
}

static bool8 game_load_saved(void) {
    SaveData data;
    const MapHeader *map;
    const MapHeader *last_map;

    if (!save_read(&data)) return FALSE;
    map = map_get_by_id(data.map_id);
    last_map = map_get_by_id(data.last_map_id);
    if (!map) return FALSE;

    for (u8 i = 0; i < SAVE_NAME_LENGTH; i++) {
        s_player_name[i] = (char)data.player_name[i];
        s_rival_name[i] = (char)data.rival_name[i];
    }
    s_player_name[SAVE_NAME_LENGTH - 1] = '\0';
    s_rival_name[SAVE_NAME_LENGTH - 1] = '\0';
    s_option_fast_text = data.option_fast_text;
    s_option_battle_animation = data.option_battle_animation;
    s_option_battle_style = data.option_battle_style;

    world_init(map, data.player_x, data.player_y);
    g_world.player.facing = (Direction)data.player_facing;
    g_world.last_map = last_map;
    flags_import(data.flags);
    return TRUE;
}

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
            s_title_mode = TITLE_MODE_PRESS_START;
            s_title_menu_cursor = 0;
            s_title_option_cursor = 0;
            s_title_has_save = title_save_available();
            audio_music_play(AUDIO_MUSIC_TITLE_SCREEN);
        } else if (entering == GAME_STATE_INTRO) {
            // The opening monologue uses the lab cue. The separate Oak
            // encounter cue starts later when Oak stops the player in Pallet.
            audio_music_play(AUDIO_MUSIC_OAKS_LAB);
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
            // A fresh game uses the temporary Red's House 2F spawn. Returning
            // from battle or the pause menu must keep the current map intact.
            if (s_continue_load) {
                s_continue_load = FALSE;
                if (!game_load_saved())
                    world_init(&g_map_reds_house_2f, 3, 3);
            } else if (previous_state != GAME_STATE_BATTLE &&
                       previous_state != GAME_STATE_MENU) {
                flags_clear_all();
                // Spawn in Red's House 2F, center of room.
                world_init(&g_map_reds_house_2f, 3, 3);
            } else if (previous_state == GAME_STATE_BATTLE) {
                // Resume map music after battle (world_init not called here).
                if (g_world.map && g_world.map->music_id)
                    audio_music_play((AudioMusicId)g_world.map->music_id);
                else
                    audio_music_stop();
            }
        } else if (entering == GAME_STATE_BATTLE) {
            battle_transition_start();
            audio_music_play(AUDIO_MUSIC_TRAINER_BATTLE);
            audio_sfx_set_battle_intro(TRUE);
        } else if (entering == GAME_STATE_MENU) {
            s_pause_menu_cursor = 0;
            s_pause_pokedex_active = FALSE;
            s_options_from_pause = FALSE;
            pause_menu_draw();
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
    if (s_title_mode == TITLE_MODE_MAIN_MENU) {
        title_menu_update();
        return;
    }
    if (s_title_mode == TITLE_MODE_OPTIONS) {
        title_options_update();
        return;
    }

    // Blink "Press START"
    if ((g_game.frame & 30) < 20) {
        text_draw_str(9, 18, "PRESS START");
    } else {
        text_draw_str(9, 18, "           ");
    }
    if (input_pressed(KEY_START) || input_pressed(KEY_A)) {
        audio_sfx_play(AUDIO_SFX_START);
        s_title_mode = TITLE_MODE_MAIN_MENU;
        s_title_has_save = title_save_available();
        s_title_menu_cursor = 0;
        title_menu_draw();
    }
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
        audio_sfx_play(AUDIO_SFX_CONFIRM);
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
        audio_sfx_play(AUDIO_SFX_CANCEL);
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

    if (moved) {
        audio_sfx_play(AUDIO_SFX_SELECT);
        intro_name_screen_draw();
    }
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
    if (input_pressed(KEY_START) && !dialog_is_open() &&
        !script_blocks_input() &&
        g_world.player.move_state == MOVE_STATE_IDLE) {
        audio_sfx_play(AUDIO_SFX_PAUSE_OPEN);
        game_change_state(GAME_STATE_MENU);
        return;
    }
    world_update();
    world_render();
}

static void state_battle_update(void) {
    battle_update();
}

static void state_menu_update(void) {
    if (s_pause_pokedex_active) {
        if (pokedex_update()) {
            pokedex_close();
            s_pause_pokedex_active = FALSE;
            pause_menu_draw();
        }
        return;
    }
    if (s_options_from_pause)
        title_options_update();
    else
        pause_menu_update();
}

static u8 pause_menu_count(void) {
    return flags_get(FLAG_GOT_POKEDEX) ? 7 : 6;
}

static void pause_menu_draw(void) {
    text_clear();
    title_draw_menu_box(14, 0, 29, 18);

    u8 row = 1;
    if (flags_get(FLAG_GOT_POKEDEX)) {
        text_draw_str(16, row, "POKeDEX");
        row = (u8)(row + 2);
    }
    text_draw_str(16, row, "POKeMON"); row = (u8)(row + 2);
    text_draw_str(16, row, "ITEM");    row = (u8)(row + 2);
    text_draw_str(16, row, s_player_name); row = (u8)(row + 2);
    text_draw_str(16, row, "SAVE");    row = (u8)(row + 2);
    text_draw_str(16, row, "OPTION");  row = (u8)(row + 2);
    text_draw_str(16, row, "EXIT");

    text_draw_char(15, (u8)(1 + s_pause_menu_cursor * 2), '>');
}

static void pause_menu_message(const char *message) {
    dialog_open();
    dialog_set_text(message);
}

static void pause_menu_select(void) {
    u8 selected = s_pause_menu_cursor;
    if (flags_get(FLAG_GOT_POKEDEX)) {
        if (selected == 0) {
            // The current Pokédex module displays a species page. Until the
            // full owned/seen database exists, use Bulbasaur as the first
            // registered entry, matching pokered's initial ordering.
            pokedex_open(POKEDEX_BULBASAUR);
            s_pause_pokedex_active = TRUE;
            return;
        }
        selected--;
    }

    switch (selected) {
    case 0:
        pause_menu_message("No other POKeMON!");
        break;
    case 1:
        pause_menu_message("No items!");
        break;
    case 2:
        pause_menu_message("ASH: [NAME]");
        break;
    case 3: {
        SaveData data;
        save_capture(&data);
        pause_menu_message(save_write(&data) ? "Game saved!" : "Save failed!");
        break;
    }
    case 4:
        s_options_from_pause = TRUE;
        s_title_option_cursor = 0;
        title_options_draw();
        break;
    default:
        text_clear();
        game_change_state(GAME_STATE_OVERWORLD);
        break;
    }
}

static void pause_menu_update(void) {
    if (dialog_is_open()) {
        if (dialog_update()) pause_menu_draw();
        return;
    }

    u8 last = (u8)(pause_menu_count() - 1);
    if (input_pressed(KEY_UP)) {
        s_pause_menu_cursor = s_pause_menu_cursor == 0
            ? last : (u8)(s_pause_menu_cursor - 1);
        pause_menu_draw();
        audio_sfx_play(AUDIO_SFX_SELECT);
        return;
    }
    if (input_pressed(KEY_DOWN)) {
        s_pause_menu_cursor = s_pause_menu_cursor >= last
            ? 0 : (u8)(s_pause_menu_cursor + 1);
        pause_menu_draw();
        audio_sfx_play(AUDIO_SFX_SELECT);
        return;
    }
    if (input_pressed(KEY_B)) {
        audio_sfx_play(AUDIO_SFX_PAUSE_CLOSE);
        text_clear();
        game_change_state(GAME_STATE_OVERWORLD);
        return;
    }
    if (input_pressed(KEY_A)) {
        audio_sfx_play(AUDIO_SFX_CONFIRM);
        pause_menu_select();
    }
}

static void title_draw_menu_box(u8 left, u8 top, u8 right, u8 bottom) {
    for (u8 y = top; y <= bottom; y++) {
        for (u8 x = left; x <= right; x++) {
            u8 tile = BOX_FILL;
            if (x == left && y == top) tile = BOX_TL;
            else if (x == right && y == top) tile = BOX_TR;
            else if (x == left && y == bottom) tile = BOX_BL;
            else if (x == right && y == bottom) tile = BOX_BR;
            else if (y == top) tile = BOX_TE;
            else if (y == bottom) tile = BOX_BE;
            else if (x == left || x == right) tile = BOX_LE;
            if (x == right && y != top && y != bottom) tile = BOX_RE;
            text_draw_tile(x, y, tile);
        }
    }
}

static void title_menu_draw(void) {
    title_hide();
    text_fill_opaque();
    title_draw_menu_box(5, 2, 24, 16);
    text_draw_str(10, 4, "MAIN MENU");

    u8 row = s_title_has_save ? (u8)(5 + s_title_menu_cursor * 2)
                              : (u8)(5 + s_title_menu_cursor * 2);
    if (s_title_has_save) {
        text_draw_str(10, 7, "CONTINUE");
        text_draw_str(10, 9, "NEW GAME");
        text_draw_str(10, 11, "OPTION");
    } else {
        text_draw_str(10, 7, "NEW GAME");
        text_draw_str(10, 9, "OPTION");
    }
    text_draw_char(8, row + 2, '>');
    text_draw_str(8, 14, "A:SELECT B:BACK");
}

static void title_menu_return_to_title(void) {
    s_title_mode = TITLE_MODE_PRESS_START;
    title_draw();
    text_clear();
    text_draw_str_pal(8, 9, "RED REMASTERED", 13);
    text_draw_str(9, 18, "PRESS START");
}

static void title_menu_update(void) {
    u8 max_cursor = s_title_has_save ? 2 : 1;

    if (input_pressed(KEY_UP)) {
        s_title_menu_cursor = s_title_menu_cursor == 0
            ? max_cursor : (u8)(s_title_menu_cursor - 1);
        title_menu_draw();
        audio_sfx_play(AUDIO_SFX_SELECT);
        return;
    }
    if (input_pressed(KEY_DOWN)) {
        s_title_menu_cursor = s_title_menu_cursor >= max_cursor
            ? 0 : (u8)(s_title_menu_cursor + 1);
        title_menu_draw();
        audio_sfx_play(AUDIO_SFX_SELECT);
        return;
    }
    if (input_pressed(KEY_B)) {
        audio_sfx_play(AUDIO_SFX_CANCEL);
        title_menu_return_to_title();
        return;
    }
    if (!input_pressed(KEY_A) && !input_pressed(KEY_START)) return;
    audio_sfx_play(AUDIO_SFX_CONFIRM);

    if (s_title_has_save && s_title_menu_cursor == 0) {
        if (save_exists()) {
            s_continue_load = TRUE;
            game_change_state(GAME_STATE_OVERWORLD);
        }
        return;
    }

    u8 selected = s_title_has_save ? (u8)(s_title_menu_cursor - 1)
                                   : s_title_menu_cursor;
    if (selected == 0) {
        // GAME_STATE_INTRO resets the opening sequence and naming state when
        // it is entered, matching pokered's StartNewGame path.
        game_change_state(GAME_STATE_INTRO);
    } else {
        s_title_mode = TITLE_MODE_OPTIONS;
        s_title_option_cursor = 0;
        title_options_draw();
    }
}

static void title_options_draw(void) {
    text_fill_opaque();
    title_draw_menu_box(4, 1, 25, 18);
    text_draw_str(10, 2, "OPTIONS");
    text_draw_str(7, 5, "TEXT SPEED");
    text_draw_str(18, 5, s_option_fast_text ? "FAST" : "MEDIUM");
    text_draw_str(7, 8, "BATTLE ANIM");
    text_draw_str(18, 8, s_option_battle_animation ? "ON" : "OFF");
    text_draw_str(7, 11, "BATTLE STYLE");
    text_draw_str(18, 11, s_option_battle_style ? "SET" : "SHIFT");
    text_draw_str(7, 14, "CANCEL");

    static const u8 rows[] = { 5, 8, 11, 14 };
    text_draw_char(5, rows[s_title_option_cursor], '>');
    text_draw_str(7, 16, "A:SELECT B:BACK");
}

static void title_options_update(void) {
    if (input_pressed(KEY_UP)) {
        s_title_option_cursor = s_title_option_cursor == 0
            ? 3 : (u8)(s_title_option_cursor - 1);
        title_options_draw();
        audio_sfx_play(AUDIO_SFX_SELECT);
        return;
    }
    if (input_pressed(KEY_DOWN)) {
        s_title_option_cursor = s_title_option_cursor >= 3
            ? 0 : (u8)(s_title_option_cursor + 1);
        title_options_draw();
        audio_sfx_play(AUDIO_SFX_SELECT);
        return;
    }
    if (input_pressed(KEY_B)) {
        audio_sfx_play(AUDIO_SFX_CANCEL);
        if (s_options_from_pause) {
            s_options_from_pause = FALSE;
            pause_menu_draw();
        } else {
            s_title_mode = TITLE_MODE_MAIN_MENU;
            title_menu_draw();
        }
        return;
    }
    if (s_title_option_cursor == 3) {
        if (input_pressed(KEY_A)) {
            audio_sfx_play(AUDIO_SFX_CONFIRM);
            if (s_options_from_pause) {
                s_options_from_pause = FALSE;
                pause_menu_draw();
            } else {
                s_title_mode = TITLE_MODE_MAIN_MENU;
                title_menu_draw();
            }
        }
        return;
    }

    if (input_pressed(KEY_LEFT) || input_pressed(KEY_RIGHT) ||
        input_pressed(KEY_A)) {
        if (s_title_option_cursor == 0) s_option_fast_text ^= 1;
        else if (s_title_option_cursor == 1) s_option_battle_animation ^= 1;
        else if (s_title_option_cursor == 2) s_option_battle_style ^= 1;
        title_options_draw();
    }
}
