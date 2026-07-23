#include "script.h"
#include "world.h"
#include "dialog.h"
#include "flags.h"
#include "battle.h"
#include "input.h"
#include "map_ids.h"
#include "pokedex.h"
#include "game.h"

// ─── Global script state ────────────────────────────────────────────────────

static bool8 s_blocks_input = FALSE;

bool8 script_blocks_input(void) { return s_blocks_input; }

// ─── NPC interaction dispatch ───────────────────────────────────────────────

// Each script_id from NpcDef maps here. Ids 1-99 are map NPCs.
// Ids 10-12 are the three Pokéballs in Oak's Lab.

static u8 s_npc_script_state = 0;
static u16 s_active_script_id = 0;
static u8 s_active_npc_index  = 0;

// Reserved script id for map-owned cutscenes. It prevents the generic NPC
// dialog dispatcher from swallowing the map script's own dialog updates.
#define ACTIVE_MAP_SCRIPT 100

static const char *const s_npc_texts[] = {
    /* 0  */ "",
    /* 1  */ "...                                     \fI've been waiting for you, [NAME]!",
    /* 2  */ "Choose a POKeMON!                       \fWhich will it be?",
    /* 3  */ "I'm raising POKeMON too!\fWhen they get\nstrong, they can\nprotect me!",
    /* 4  */ "Technology is incredible!\fYou can now store\nand recall items\nand POKeMON as\ndata via PC!",
    /* 5  */ "Hi [NAME]!\n[RIVAL] is out at\nGrandpa's lab.",
    /* 6  */ "POKeMON are living things!\fIf they get tired, give\nthem a rest!",
    /* 7  */ "It's a big map!\fThis is useful!",
    /* 8  */ "MOM: Right.\nAll boys leave\nhome some day.\nIt said so on TV.\fPROF.OAK, next\ndoor, is looking\nfor you.",
};

void script_trigger_npc(u16 script_id, u8 npc_index) {
    if (s_blocks_input) return;
    if (dialog_is_open()) return;

    s_active_script_id = script_id;
    s_active_npc_index  = npc_index;

    if (script_id >= 10 && script_id <= 12) {
        // Pokéball interaction handled by oaks_lab script
        s_npc_script_state = 1;
        s_blocks_input = TRUE;
        return;
    }

    if (script_id < ARRAY_COUNT(s_npc_texts)) {
        const char *text = s_npc_texts[script_id];
        if (text[0]) {
            dialog_open();
            dialog_set_text(text);
            s_npc_script_state = 1;
            s_blocks_input = TRUE;
        }
    }
}

// Returns TRUE when NPC dialog is done.
static bool8 npc_script_tick(void) {
    if (!s_blocks_input) return TRUE;

    if (s_active_script_id >= 10 && s_active_script_id <= 12) {
        // Pokéball: handled by oaks_lab script — it clears s_blocks_input
        return FALSE;
    }

    if (s_npc_script_state == 1) {
        if (dialog_update()) {
            // Stay blocked for one more frame so the A press that closed the
            // dialog isn't seen by player_update() as a fresh NPC interaction.
            s_npc_script_state = 2;
        }
        return FALSE;
    }
    if (s_npc_script_state == 2) {
        s_npc_script_state = 0;
        s_blocks_input = FALSE;
        return TRUE;
    }
    return FALSE;
}

void script_update(void) {
    // Map-owned cutscenes use ids >= ACTIVE_MAP_SCRIPT and are advanced by
    // their map script. Generic NPC dialogs must be serviced globally so
    // indoor maps without a map script work the same way as Pallet Town.
    if (s_blocks_input && s_active_script_id < 10)
        npc_script_tick();
}

// ─── Pallet Town script ─────────────────────────────────────────────────────

typedef enum {
    PT_IDLE = 0,
    PT_OAK_FIRST_TEXT,
    PT_OAK_WALK_TO_PLAYER,
    PT_OAK_SECOND_TEXT,
    PT_OAK_LEADS_PLAYER,
} PalletScriptState;

static PalletScriptState s_pallet_state = PT_IDLE;
static u8 s_pallet_route_step = 0;
static u8 s_pallet_player_route_step = 0;
static Direction s_pallet_player_route[64];
static u8 s_pallet_player_route_count = 0;
static bool8 s_pallet_player_pending = FALSE;
static bool8 s_pallet_lab_flag_set = FALSE;
static u16 s_pallet_move_watchdog = 0;

// This is the movement sequence from pokered's
// PalletMovementScript_WalkToLab. Oak and the player have separate scripts in
// the original game; they must not share one guessed path.
static const Direction s_pallet_oak_route[] = {
    DIR_DOWN, DIR_DOWN, DIR_DOWN, DIR_DOWN, DIR_DOWN,
    DIR_LEFT,
    DIR_DOWN, DIR_DOWN, DIR_DOWN, DIR_DOWN, DIR_DOWN,
    DIR_RIGHT, DIR_RIGHT, DIR_RIGHT,
    DIR_UP,
};

static bool8 pallet_scene_tile_is_walkable(s16 x, s16 y) {
    // Red's house / left tree block
    if (x >= 4 && x <= 7 && y >= 2 && y <= 11) return FALSE;
    // Right side (Blue's house + lab body) above the door row
    if (x >= 12 && x <= 15 && y >= 2 && y <= 9) return FALSE;
    // Lab solid face (top two subtile rows of the building)
    if (x >= 10 && x <= 15 && y >= 8 && y <= 9) return FALSE;
    // Grass-step tile to the right of the door
    if (x >= 14 && x <= 15 && y >= 10 && y <= 11) return FALSE;
    return TRUE;
}

static void build_pallet_player_fallback_path(void) {
    const PlayerState *p = &g_world.player;
    u8 count = 0;
    s16 x = p->tile_x;
    s16 y = p->tile_y;

    // The trigger fires at y=1. At y<=1, x=4-7 is not yet blocked (rule 1
    // starts at y=2), so we can steer right to x=8 — the open road between
    // the two houses — from any starting column.
    if (y <= 1) {
        while (x < 8 && count < ARRAY_COUNT(s_pallet_player_route)) {
            s_pallet_player_route[count++] = DIR_RIGHT;
            x++;
        }
        while (x >= 10 && count < ARRAY_COUNT(s_pallet_player_route)) {
            s_pallet_player_route[count++] = DIR_LEFT;
            x--;
        }
    }

    // Walk south to the lab-door row (y=11). x=8-9 is clear of all rules
    // down to y=11, and the dirt path at x=10-11 is passable at y=10-11.
    while (y < 11 && count < ARRAY_COUNT(s_pallet_player_route)) {
        s_pallet_player_route[count++] = DIR_DOWN;
        y++;
    }

    // Walk east along the door row into (12,11).
    while (x < 12 && count < ARRAY_COUNT(s_pallet_player_route)) {
        s_pallet_player_route[count++] = DIR_RIGHT;
        x++;
    }
    while (x > 12 && count < ARRAY_COUNT(s_pallet_player_route)) {
        s_pallet_player_route[count++] = DIR_LEFT;
        x--;
    }

    s_pallet_player_route_count = count;
}

void script_pallet_town(void) {
    PlayerState *p = &g_world.player;

    switch (s_pallet_state) {
    case PT_IDLE:
        // pokered's PalletTownDefaultScript fires on the exact north-exit row.
        if (flags_get(FLAG_FOLLOWED_OAK_INTO_LAB)) break;
        if (p->tile_y != 1) break;
        if (!flags_get(FLAG_OAK_APPEARED_IN_PALLET)) {
            flags_set(FLAG_OAK_APPEARED_IN_PALLET);
            p->move_state = MOVE_STATE_FROZEN;
            s_blocks_input = TRUE;
            s_active_script_id = ACTIVE_MAP_SCRIPT;
            g_world.npcs[0].flags &= (u8)~NPCF_HIDDEN;
            g_world.npcs[0].facing = DIR_UP;
            dialog_open();
            dialog_set_text("OAK: Hey! Wait!          \fDon't go out!");
            s_pallet_state = PT_OAK_FIRST_TEXT;
        }
        break;

    case PT_OAK_FIRST_TEXT:
        if (dialog_update()) {
            s_pallet_state = PT_OAK_WALK_TO_PLAYER;
        }
        break;

    case PT_OAK_WALK_TO_PLAYER: {
        NpcState *oak = &g_world.npcs[0];
        if (++s_pallet_move_watchdog > 300) {
            oak->x = (u8)p->tile_x;
            oak->y = (u8)(p->tile_y + 1);
            oak->px = (s16)oak->x * 16;
            oak->py = (s16)oak->y * 16;
            oak->walking = FALSE;
            s_pallet_move_watchdog = 0;
        }
        if (world_npc_is_moving(0)) break;

        // This project uses the corrected interaction-tile sprite anchor, so
        // Oak begins the pokered WalkToLab route one logical tile below the
        // player rather than two tiles below the player.
        s16 target_x = p->tile_x;
        s16 target_y = p->tile_y + 1;
        if (oak->x != target_x) {
            world_npc_start_step(0, oak->x < target_x ? DIR_RIGHT : DIR_LEFT);
        } else if (oak->y != target_y) {
            world_npc_start_step(0, oak->y < target_y ? DIR_DOWN : DIR_UP);
        } else {
            oak->facing = DIR_DOWN;
            s_pallet_move_watchdog = 0;
            dialog_open();
            dialog_set_text("OAK: It's unsafe!         \fWild POKeMON live in   tall grass!\fYou need your own       POKeMON for your      protection. I know!\fHere, come with me!");
            s_pallet_state = PT_OAK_SECOND_TEXT;
        }
        break;
    }

    case PT_OAK_SECOND_TEXT:
        if (dialog_update()) {
            // Set the flag now — the player has agreed to follow Oak.
            // This must happen before the walk starts so player_check_warps()
            // can't fire on the door tile before we get to set it.
            flags_set(FLAG_FOLLOWED_OAK_INTO_LAB);
            s_pallet_route_step = 0;
            s_pallet_player_route_step = 0;
            build_pallet_player_fallback_path();
            s_pallet_player_pending = FALSE;
            s_pallet_lab_flag_set = FALSE;
            s_pallet_move_watchdog = 0;
            s_pallet_state = PT_OAK_LEADS_PLAYER;
        }
        break;

    case PT_OAK_LEADS_PLAYER:
        if (++s_pallet_move_watchdog > 600) {
            // Never leave the player permanently locked if a scripted step
            // is interrupted by a map edge or an unexpected tile state.
            s_pallet_move_watchdog = 0;
            s_pallet_player_pending = FALSE;
            s_pallet_route_step = ARRAY_COUNT(s_pallet_oak_route);
            s_pallet_player_route_step = s_pallet_player_route_count;
            if (g_world.map->map_id == MAP_PALLET_TOWN) {
                flags_set(FLAG_FOLLOWED_OAK_INTO_LAB);
                s_blocks_input = FALSE;
                world_do_warp(&g_world.map->warps[2]);
            }
            break;
        }
        if (world_npc_is_moving(0) || player_script_is_moving()) break;

        if (s_pallet_route_step < ARRAY_COUNT(s_pallet_oak_route)) {
            world_npc_start_step(0, s_pallet_oak_route[s_pallet_route_step++]);
        }
        if (s_pallet_player_route_step < s_pallet_player_route_count) {
            Direction dir = s_pallet_player_route[s_pallet_player_route_step];
            s16 next_x = g_world.player.tile_x +
                         ((dir == DIR_RIGHT) ? 1 : (dir == DIR_LEFT) ? -1 : 0);
            s16 next_y = g_world.player.tile_y +
                         ((dir == DIR_DOWN) ? 1 : (dir == DIR_UP) ? -1 : 0);
            if (pallet_scene_tile_is_walkable(next_x, next_y) &&
                player_script_start_step_forced(dir))
                s_pallet_player_route_step++;
            else {
                build_pallet_player_fallback_path();
                s_pallet_player_route_step = 0;
            }
        }
        if (s_pallet_route_step == ARRAY_COUNT(s_pallet_oak_route)) {
            flags_set(FLAG_FOLLOWED_OAK_INTO_LAB);
            s_pallet_lab_flag_set = TRUE;
        }
        if (s_pallet_player_route_step < s_pallet_player_route_count)
            break;
        // Wait for the final walking animation to finish so player_check_warps()
        // fires naturally on the door tile (12,11).
        if (player_script_is_moving())
            break;

        // Fallback: if player_check_warps() didn't trigger the warp (e.g. non-
        // standard passability flags), fire it explicitly here.
        if (s_pallet_lab_flag_set && g_world.map->map_id == MAP_PALLET_TOWN) {
            const WarpEvent *w = &g_world.map->warps[2];
            s_pallet_lab_flag_set = FALSE;
            s_blocks_input = FALSE;
            world_do_warp(w);
        }
        break;

    }
}

// ─── Oak's Lab script ───────────────────────────────────────────────────────

typedef enum {
    OAKSLAB_IDLE = 0,
    OAKSLAB_OAK_ENTRY,        // Oak2 walks UP x3 from (5,10) to (5,7)
    OAKSLAB_PLAYER_ENTRY,     // player walks UP x8 from (5,11) to (5,3)
    OAKSLAB_INTRO_RIVAL,
    OAKSLAB_INTRO_OAK,
    OAKSLAB_INTRO_RIVAL_2,
    OAKSLAB_INTRO_OAK_2,
    OAKSLAB_WAIT_CHOOSE,      // player can press A on pokeballs
    OAKSLAB_POKEDEX_WAIT,     // Pokédex entry shown before the choice prompt
    OAKSLAB_YESNO_WAIT,       // YES/NO menu for chosen pokemon
    OAKSLAB_NICKNAME_YESNO,   // ask whether to nickname the starter
    OAKSLAB_NICKNAME_SCREEN,  // pokered-style naming screen
    OAKSLAB_RIVAL_MOVE,       // rival walks to the opposite starter ball
    OAKSLAB_CHOSE_STARTER,    // set flag, rival dialog
    OAKSLAB_WAIT_BATTLE_TRIGGER, // player walks to the battle position
    OAKSLAB_RIVAL_APPROACH,   // rival walks next to the player
    OAKSLAB_RIVAL_CHALLENGE,  // rival fights player
    OAKSLAB_DONE,
} OaksLabScriptState;

static OaksLabScriptState s_oakslab_state = OAKSLAB_IDLE;
static u16 s_chosen_ball = 0; // script_id of selected pokeball (10/11/12)
static u8 s_oakslab_oak_step = 0;
static u8 s_oakslab_player_step = 0;
static char s_starter_nickname[8];
static u8 s_rival_target_npc = 0;
static bool8 s_oakslab_battle_row_armed = FALSE;

static const char *s_ball_names[] = {
    /* 10 */ "CHARMANDER",
    /* 11 */ "SQUIRTLE",
    /* 12 */ "BULBASAUR",
};

void script_reset_runtime(void) {
    s_blocks_input = FALSE;
    s_npc_script_state = 0;
    s_active_script_id = 0;
    s_active_npc_index = 0;
    s_pallet_state = PT_IDLE;
    s_pallet_route_step = 0;
    s_pallet_player_route_step = 0;
    s_pallet_player_route_count = 0;
    s_pallet_player_pending = FALSE;
    s_pallet_lab_flag_set = FALSE;
    s_pallet_move_watchdog = 0;
    s_oakslab_state = OAKSLAB_IDLE;
    s_chosen_ball = 0;
    s_oakslab_oak_step = 0;
    s_oakslab_player_step = 0;
    s_starter_nickname[0] = '\0';
    s_rival_target_npc = 0;
    s_oakslab_battle_row_armed = FALSE;
}

static void oaks_lab_start_rival_choice(void) {
    // pokered's opposite starter mapping:
    // Charmander -> Squirtle, Squirtle -> Bulbasaur, Bulbasaur -> Charmander.
    if (s_chosen_ball == 10) s_rival_target_npc = 4;      // right ball
    else if (s_chosen_ball == 11) s_rival_target_npc = 3; // middle ball
    else s_rival_target_npc = 2;                          // left ball
    s_oakslab_state = OAKSLAB_RIVAL_MOVE;
}

// Find the first step of a walkable route through Oak's Lab.  NPC movement
// itself is intentionally permissive for cutscenes, so the route must do the
// collision checking before each step; otherwise a greedy horizontal move can
// enter one of the bookshelf rows and leave the rival stranded.
static bool8 oaks_lab_find_rival_step(s16 start_x, s16 start_y,
                                      s16 target_x, s16 target_y,
                                      Direction *out_dir) {
    enum { GRID_W = 15, GRID_H = 12, GRID_SIZE = GRID_W * GRID_H };
    static u8 queue[GRID_SIZE];
    static u8 visited[GRID_SIZE];
    static u8 parent[GRID_SIZE];
    static u8 parent_dir[GRID_SIZE];
    static const s8 dx[4] = { 0, 0, -1, 1 };
    static const s8 dy[4] = { 1, -1, 0, 0 };
    u8 head = 0, tail = 0;
    s16 current;

    if (start_x < 0 || start_x >= GRID_W || start_y < 0 || start_y >= GRID_H ||
        target_x < 0 || target_x >= GRID_W || target_y < 0 || target_y >= GRID_H)
        return FALSE;
    if (start_x == target_x && start_y == target_y)
        return FALSE;

    for (u16 i = 0; i < GRID_SIZE; i++) {
        visited[i] = FALSE;
        parent[i] = 0xFF;
    }

    current = (s16)(start_y * GRID_W + start_x);
    queue[tail++] = (u8)current;
    visited[current] = TRUE;

    while (head != tail) {
        current = queue[head++];
        s16 cx = current % GRID_W;
        s16 cy = current / GRID_W;
        if (cx == target_x && cy == target_y) break;

        for (u8 dir = 0; dir < 4; dir++) {
            s16 nx = cx + dx[dir];
            s16 ny = cy + dy[dir];
            if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H)
                continue;
            s16 next = ny * GRID_W + nx;
            if (visited[next] ||
                (!map_is_subtile_passable(nx, ny) &&
                 !(nx == target_x && ny == target_y)))
                continue;
            visited[next] = TRUE;
            parent[next] = (u8)current;
            parent_dir[next] = dir;
            queue[tail++] = (u8)next;
        }
    }

    current = (s16)(target_y * GRID_W + target_x);
    if (!visited[current]) return FALSE;
    // Walk backwards from the destination until its first child of the
    // starting tile is found; that child contains the first direction.
    while (parent[current] != (u8)((start_y * GRID_W) + start_x))
        current = parent[current];
    *out_dir = (Direction)parent_dir[current];
    return TRUE;
}

void script_oaks_lab(void) {
    // NPC dialog ticking (non-pokeball NPCs)
    if (s_blocks_input && s_active_script_id < 10) {
        npc_script_tick();
        return;
    }

    switch (s_oakslab_state) {
    case OAKSLAB_IDLE:
        if (!flags_get(FLAG_FOLLOWED_OAK_INTO_LAB)) break;
        if (flags_get(FLAG_OAK_ASKED_TO_CHOOSE_MON)) {
            // Re-entry after first time: Oak2 already walked in, fix NPC visibility
            g_world.npcs[5].flags |= NPCF_HIDDEN;
            g_world.npcs[1].flags &= (u8)~NPCF_HIDDEN;
            s_oakslab_state = OAKSLAB_WAIT_CHOOSE;
            break;
        }
        // First-time entry: freeze player, begin Oak2 walk-in animation
        g_world.player.move_state = MOVE_STATE_FROZEN;
        s_blocks_input = TRUE;
        s_active_script_id = ACTIVE_MAP_SCRIPT;
        s_oakslab_state = OAKSLAB_OAK_ENTRY;
        break;

    case OAKSLAB_OAK_ENTRY:
        if (world_npc_is_moving(5)) break;
        if (s_oakslab_oak_step < 3) {
            world_npc_start_step(5, DIR_UP);
            s_oakslab_oak_step++;
            break;
        }
        // Oak2 reached position: hide Oak2, reveal Oak1, both face player
        g_world.npcs[5].flags |= NPCF_HIDDEN;
        g_world.npcs[1].flags &= (u8)~NPCF_HIDDEN;
        g_world.npcs[0].facing = DIR_DOWN;
        g_world.npcs[1].facing = DIR_DOWN;
        s_oakslab_state = OAKSLAB_PLAYER_ENTRY;
        break;

    case OAKSLAB_PLAYER_ENTRY:
        if (player_script_is_moving()) break;
        if (s_oakslab_player_step < 8) {
            player_script_start_step_forced(DIR_UP);
            s_oakslab_player_step++;
            break;
        }
        // Player arrived: Rival turns to face Oak, start dialog sequence
        g_world.npcs[0].facing = DIR_UP;
        dialog_open();
        dialog_set_text("RIVAL: Gramps!\fI'm fed up with\nwaiting!");
        s_oakslab_state = OAKSLAB_INTRO_RIVAL;
        break;

    case OAKSLAB_INTRO_RIVAL:
        if (dialog_update()) {
            dialog_open();
            dialog_set_text("OAK: [RIVAL]? Let me think...\fOh, that's right, I told you to come! Just wait!\fHere, [NAME]! There are 3 POKeMON here!\fThey are inside the POKeBALLs.\fWhen I was young, I was a serious POKeMON trainer!\fIn my old age, I have only 3 left, but you can have one!\fChoose!");
            s_oakslab_state = OAKSLAB_INTRO_OAK;
        }
        break;

    case OAKSLAB_INTRO_OAK:
        if (dialog_update()) {
            dialog_open();
            dialog_set_text("RIVAL: Hey! Gramps! What\nabout me?");
            s_oakslab_state = OAKSLAB_INTRO_RIVAL_2;
        }
        break;

    case OAKSLAB_INTRO_RIVAL_2:
        if (dialog_update()) {
            dialog_open();
            dialog_set_text("OAK: Be patient!\f[RIVAL], you can have one too!");
            s_oakslab_state = OAKSLAB_INTRO_OAK_2;
        }
        break;

    case OAKSLAB_INTRO_OAK_2:
        if (dialog_update()) {
            flags_set(FLAG_OAK_ASKED_TO_CHOOSE_MON);
            g_world.player.move_state = MOVE_STATE_IDLE;
            s_blocks_input = FALSE;
            s_oakslab_state = OAKSLAB_WAIT_CHOOSE;
        }
        break;

    case OAKSLAB_WAIT_CHOOSE:
        if (flags_get(FLAG_GOT_STARTER)) {
            s_oakslab_state = OAKSLAB_DONE;
            break;
        }
        // Pokéball NPC triggered?
        if (s_blocks_input && s_active_script_id >= 10 && s_active_script_id <= 12) {
            s_chosen_ball = s_active_script_id;
            PokedexSpecies species = POKEDEX_BULBASAUR;
            if (s_chosen_ball == 10) species = POKEDEX_CHARMANDER;
            else if (s_chosen_ball == 11) species = POKEDEX_SQUIRTLE;
            pokedex_open(species);
            s_oakslab_state = OAKSLAB_POKEDEX_WAIT;
        }
        break;

    case OAKSLAB_POKEDEX_WAIT:
        if (pokedex_update()) {
            u8 idx = (u8)(s_chosen_ball - 10);
            pokedex_close();
            dialog_open();
            dialog_yesno_open();
            dialog_set_text(s_ball_names[idx]);
            s_oakslab_state = OAKSLAB_YESNO_WAIT;
        }
        break;

    case OAKSLAB_YESNO_WAIT: {
        u8 result = dialog_yesno_update();
        if (result == 0xFF) break; // pending
        s_blocks_input = FALSE;
        if (result == 1) {
            // YES — took the starter (CHOSE_STARTER will open its own dialog)
            s_oakslab_state = OAKSLAB_CHOSE_STARTER;
        } else {
            // NO — close the Pokémon-name dialog that WAIT_CHOOSE opened,
            // otherwise dialog_is_open() keeps player_update returning early.
            dialog_close();
            // Consume the A press used to choose NO before returning control
            // to the player, just like backing out with B.
            s_active_script_id = 0;
            s_npc_script_state = 1;
            s_blocks_input = TRUE;
            s_oakslab_state = OAKSLAB_WAIT_CHOOSE;
        }
        break;
    }

    case OAKSLAB_CHOSE_STARTER:
        flags_set(FLAG_GOT_STARTER);
        g_world.npcs[s_active_npc_index].flags |= NPCF_HIDDEN;
        s_blocks_input = TRUE;
        dialog_open();
        dialog_yesno_open();
        dialog_set_text("Do you want to give a\nnickname to this POKeMON?");
        s_oakslab_state = OAKSLAB_NICKNAME_YESNO;
        break;

    case OAKSLAB_NICKNAME_YESNO: {
        u8 result = dialog_yesno_update();
        if (result == 0xFF) break;
        dialog_close();
        if (result == 1) {
            u8 idx = (u8)(s_chosen_ball - 10);
            game_nickname_open(s_ball_names[idx]);
            s_oakslab_state = OAKSLAB_NICKNAME_SCREEN;
        } else {
            oaks_lab_start_rival_choice();
        }
        break;
    }

    case OAKSLAB_NICKNAME_SCREEN:
        if (!game_nickname_active()) {
            const char *nickname = game_get_nickname_result();
            for (u8 i = 0; i < sizeof(s_starter_nickname) - 1; i++) {
                s_starter_nickname[i] = nickname[i];
                if (!nickname[i]) break;
            }
            s_starter_nickname[sizeof(s_starter_nickname) - 1] = '\0';
            oaks_lab_start_rival_choice();
        }
        break;

    case OAKSLAB_RIVAL_MOVE: {
        NpcState *rival = &g_world.npcs[0];
        NpcState *target = &g_world.npcs[s_rival_target_npc];
        // The rival stands on the floor tile below the ball, not on the
        // table tile occupied by the ball itself.
        s16 target_x = target->x;
        s16 target_y = (s16)target->y + 1;
        Direction route_dir;
        if (world_npc_is_moving(0)) break;
        if (rival->x != target_x || rival->y != target_y) {
            if (oaks_lab_find_rival_step(rival->x, rival->y,
                                          target_x, target_y, &route_dir))
                world_npc_start_step(0, route_dir);
            break;
        } else {
            target->flags |= NPCF_HIDDEN;
            rival->facing = DIR_UP;
            dialog_open();
            dialog_set_text("RIVAL: Hey! That's the one  I wanted!\fI'll take this one then!");
            s_oakslab_state = OAKSLAB_RIVAL_CHALLENGE;
        }
        break;
    }

    case OAKSLAB_RIVAL_CHALLENGE:
        if (dialog_update()) {
            // The player must make this walk themselves.  The original
            // sequence waits for the player to reach Y=6 before the rival
            // turns and starts the challenge.
            s_blocks_input = FALSE;
            g_world.player.move_state = MOVE_STATE_IDLE;
            s_oakslab_battle_row_armed = (g_world.player.tile_y != 6);
            // The rival's battle approach begins at the ball they selected.
            // Keep this invariant even if the earlier ball-selection walk was
            // interrupted by a map redraw or a stale NPC movement frame.
            NpcState *target = &g_world.npcs[s_rival_target_npc];
            NpcState *rival = &g_world.npcs[0];
            rival->x = target->x;
            rival->y = (u8)(target->y + 1);
            rival->px = (s16)rival->x * 16;
            rival->py = (s16)rival->y * 16;
            rival->walking = FALSE;
            target->flags |= NPCF_HIDDEN;
            s_oakslab_state = OAKSLAB_WAIT_BATTLE_TRIGGER;
        }
        break;

    case OAKSLAB_WAIT_BATTLE_TRIGGER:
        // Let normal player input move the player through the lower aisle.
        // This is the red-square area in the reference, between the lower
        // bookcases, rather than the upper blue-X area.  The trigger is one
        // row above the previously used position.
        if (g_world.player.tile_x < 3 || g_world.player.tile_x > 10 ||
            g_world.player.tile_y < 8 || g_world.player.tile_y > 9) {
            s_oakslab_battle_row_armed = TRUE;
            break;
        }
        if (!s_oakslab_battle_row_armed) break;

        s_blocks_input = TRUE;
        s_active_script_id = 10;
        s_oakslab_state = OAKSLAB_RIVAL_APPROACH;
        break;

    case OAKSLAB_RIVAL_APPROACH: {
        NpcState *rival = &g_world.npcs[0];
        const PlayerState *player = &g_world.player;
        s16 dx = player->tile_x - rival->x;
        s16 dy = player->tile_y - rival->y;
        // Reference route: leave the ball to the left, then come down the
        // open aisle before moving across to the player.
        // The aisle is fixed in the room; it must not shift with the selected
        // Pokéball.  X=5 is the centered gap shown by the correct Charmander
        // route and is shared by all three rival choices.
        s16 aisle_x = 5;
        if (aisle_x < 1) aisle_x = 1;

        if (world_npc_is_moving(0) || player_script_is_moving()) break;

        // Stop one tile away from the player, matching the reference's
        // face-to-face battle setup.  Do not cut diagonally through the
        // bookshelf row: horizontal movement ends at the aisle first.
        if (rival->x < aisle_x)
            world_npc_start_step(0, DIR_RIGHT);
        else if (rival->x > aisle_x)
            world_npc_start_step(0, DIR_LEFT);
        else if (dy > 1)
            world_npc_start_step(0, DIR_DOWN);
        else if (dy < -1)
            world_npc_start_step(0, DIR_UP);
        else if (dx > 1)
            world_npc_start_step(0, DIR_RIGHT);
        else if (dx < -1)
            world_npc_start_step(0, DIR_LEFT);
        else {
            if (player->tile_x < rival->x) rival->facing = DIR_LEFT;
            else if (player->tile_x > rival->x) rival->facing = DIR_RIGHT;
            else if (player->tile_y < rival->y) rival->facing = DIR_UP;
            else rival->facing = DIR_DOWN;
            dialog_open();
            dialog_set_text("RIVAL: I'll take you on,   [NAME]! Prepare yourself!");
            s_oakslab_state = OAKSLAB_DONE;
        }
        break;
    }

    case OAKSLAB_DONE:
        // battle_end() sets this flag before returning to the lab. Keep the
        // completed scene idle instead of starting the rival battle again.
        if (flags_get(FLAG_BATTLED_RIVAL_IN_OAKS_LAB)) {
            s_blocks_input = FALSE;
            break;
        }
        if (!dialog_is_open()) {
            s_blocks_input = FALSE;
            g_world.player.move_state = MOVE_STATE_IDLE;
            battle_setup_rival(s_chosen_ball, s_starter_nickname);
            game_change_state(GAME_STATE_BATTLE);
        } else {
            dialog_update();
        }
        break;
    }
}
