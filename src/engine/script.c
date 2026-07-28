#include "script.h"
#include "world.h"
#include "dialog.h"
#include "flags.h"
#include "battle.h"
#include "input.h"
#include "map_ids.h"
#include "pokedex.h"
#include "game.h"
#include "audio.h"
#include "party.h"
#include "item.h"
#include "text.h"
#include "pc.h"

// ─── Global script state ────────────────────────────────────────────────────

static bool8 s_blocks_input = FALSE;

bool8 script_blocks_input(void) { return s_blocks_input; }

// ─── NPC interaction dispatch ───────────────────────────────────────────────

// Each script_id from NpcDef maps here. Ids 1-99 are map NPCs.
// Ids 10-12 are the three Pokéballs in Oak's Lab.

static u8 s_npc_script_state = 0;
static u16 s_active_script_id = 0;
static u8 s_active_npc_index  = 0;
static bool8 s_active_trainer_battle = FALSE;
static u8 s_route1_youngster_state = 0;
static char s_item_pickup_text[64];

// Reserved script id for the pokered trainer flow: approach, show battle
// text, then initialize the battle after the text box is dismissed.
#define ACTIVE_TRAINER_DIALOG 101

static void start_active_trainer_battle(void) {
    if (!g_world.map || s_active_npc_index >= g_world.npc_count)
        return;
    NpcState *trainer = &g_world.npcs[s_active_npc_index];
    PartyPokemon *lead = party_get_lead();
    const char *nickname = lead && lead->nickname[0] ? lead->nickname : NULL;
    battle_setup_trainer_variant((TrainerId)trainer->trainer_id,
                                 trainer->trainer_party, nickname);
    audio_music_play(AUDIO_MUSIC_TRAINER_BATTLE);
    battle_transition_start();
    s_blocks_input = FALSE;
    game_change_state(GAME_STATE_BATTLE);
}

static void item_text_build(const char *prefix, const char *name) {
    u8 i = 0;
    while (prefix[i] && i + 1 < sizeof(s_item_pickup_text)) {
        s_item_pickup_text[i] = prefix[i];
        i++;
    }
    while (name && name[0] && i + 1 < sizeof(s_item_pickup_text)) {
        s_item_pickup_text[i++] = *name++;
    }
    if (i + 1 < sizeof(s_item_pickup_text))
        s_item_pickup_text[i++] = '!';
    s_item_pickup_text[i] = '\0';
}

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
    /* 9  */ "PROF.OAK is the\nauthority on\nPOKeMON!\fMany POKeMON\ntrainers hold him\nin high regard!",
    /* 10 */ "",
    /* 11 */ "",
    /* 12 */ "",
    /* 13 */ "I study POKeMON as\nPROF.OAK's AIDE.",
    /* 14 */ "I study POKeMON as\nPROF.OAK's AIDE.",
    /* 15 */ "Those POKeBALLs at\nyour waist!\nYou have POKeMON!\fIt's great that you\ncan carry and use\nPOKeMON any time,\nanywhere!",
    /* 16 */ "This POKeMON GYM is\nalways closed.\fI wonder who the\nLEADER is?",
    /* 17 */ "CATERPIE has no\npoison, but WEEDLE\ndoes.\fWatch out for its\nPOISON STING!",
    /* 18 */ "Oh Grandpa! Don't\nbe so mean!\nHe hasn't had his\ncoffee yet.",
    /* 19 */ "You can't go\nthrough here!\fThis is private\nproperty!",
    /* 20 */ "Yawn!\nI must have dozed\noff in the sun.\fI had this dream\nabout a DROWZEE\neating my dream.\nWhat's this?\fThis is spooky!\nHere, you can have\nthis TM.",
    /* 21 */ "Ahh, I've had my\ncoffee now and I\nfeel great!\fSure you can go\nthrough!\fLet me show you\nhow to catch a\nPOKeMON!",
    /* 22 */ "Okay! Say hi to\nPROF.OAK for me!",
    /* 23 */ "This shop sells\nmany ANTIDOTEs.",
    /* 24 */ "No! POTIONs are\nall sold out.",
    /* 25 */ "Welcome to our\nPOKeMON CENTER!\fWe heal your\nPOKeMON back to\nperfect health!",
    /* 26 */ "You can use that PC\nin the corner.\fThe receptionist\ntold me. So kind!",
    /* 27 */ "There's a POKeMON\nCENTER in every\ntown ahead.\fThey don't charge\nany money either!",
    /* 28 */ "Welcome!\fThe CABLE CLUB is\nupstairs.",
    /* 29 */ "Whew! I'm trying\nto memorize all\nmy notes.",
    /* 30 */ "Okay! Be sure to\nread the blackboard\ncarefully!",
    /* 31 */ "Coming up with\nnicknames is fun,\nbut hard.\fSimple names are\nthe easiest to\nremember.",
    /* 32 */ "My Daddy loves\nPOKeMON too.",
    /* 33 */ "SPEARY: Tetweet!",
    /* 34 */ "",
    /* 35 */ "Yo! Champ in making!\fThis GYM's door is\nlocked right now.",
    /* 36 */ "",
    /* 37 */ "",
    /* 38 */ "I just saw your\nPOKeMON! They look\nreally strong!",
    /* 39 */ "You need all eight\nBADGEs to get\nthrough here.\fCome back when\nyou have them!",
};

static u8 s_viridian_mart_state = 0;
static bool8 s_oak_pokedex_pending = FALSE;

// ─── Shop system ────────────────────────────────────────────────────────────

typedef enum {
    SHOP_IDLE = 0,
    SHOP_WELCOME,
    SHOP_MENU,
    SHOP_BUY_SELECT,
    SHOP_BUY_CONFIRM,
    SHOP_BUY_DONE,
    SHOP_GOODBYE,
} ShopState;

static ShopState s_shop_state = SHOP_IDLE;
static u8 s_shop_cursor = 0;

typedef struct {
    ItemId id;
    u16 price;
} ShopItem;

static const ShopItem s_viridian_shop[] = {
    { ITEM_POKE_BALL,   200 },
    { ITEM_ANTIDOTE,    100 },
    { ITEM_PARLYZ_HEAL, 200 },
    { ITEM_BURN_HEAL,   250 },
};
#define VIRIDIAN_SHOP_COUNT 4

static void shop_draw_box(u8 left, u8 top, u8 right, u8 bottom) {
    for (u8 y = top; y <= bottom; y++) {
        for (u8 x = left; x <= right; x++) {
            u8 tile = BOX_FILL;
            if (x == left && y == top) tile = BOX_TL;
            else if (x == right && y == top) tile = BOX_TR;
            else if (x == left && y == bottom) tile = BOX_BL;
            else if (x == right && y == bottom) tile = BOX_BR;
            else if (y == top) tile = BOX_TE;
            else if (y == bottom) tile = BOX_BE;
            else if (x == left) tile = BOX_LE;
            else if (x == right) tile = BOX_RE;
            text_draw_tile(x, y, tile);
        }
    }
}

static void shop_draw_money(void) {
    shop_draw_box(0, 0, 10, 2);
    char buf[12] = "$";
    u32 m = g_player_money;
    char *p = buf + 1;
    if (m >= 100000) *p++ = (char)('0' + m / 100000 % 10);
    if (m >= 10000) *p++ = (char)('0' + m / 10000 % 10);
    if (m >= 1000) *p++ = (char)('0' + m / 1000 % 10);
    if (m >= 100) *p++ = (char)('0' + m / 100 % 10);
    if (m >= 10) *p++ = (char)('0' + m / 10 % 10);
    *p++ = (char)('0' + m % 10);
    *p = '\0';
    text_draw_str(1, 1, buf);
}

static void shop_draw_menu(void) {
    text_clear();
    shop_draw_money();
    shop_draw_box(14, 0, 29, 4);
    text_draw_str(16, 1, "BUY");
    text_draw_str(16, 3, "SEE YA!");
    text_draw_char(15, (u8)(1 + s_shop_cursor * 2), '>');
}

static void shop_draw_items(void) {
    text_clear();
    shop_draw_money();
    shop_draw_box(0, 3, 29, 19);
    for (u8 i = 0; i < VIRIDIAN_SHOP_COUNT; i++) {
        u8 row = (u8)(5 + i * 2);
        text_draw_str(3, row, item_get_name(s_viridian_shop[i].id));
        char price[8] = "$";
        char *p = price + 1;
        u16 pr = s_viridian_shop[i].price;
        if (pr >= 1000) *p++ = (char)('0' + pr / 1000 % 10);
        if (pr >= 100) *p++ = (char)('0' + pr / 100 % 10);
        if (pr >= 10) *p++ = (char)('0' + pr / 10 % 10);
        *p++ = (char)('0' + pr % 10);
        *p = '\0';
        text_draw_str(20, row, price);
    }
    u8 cancel_row = (u8)(5 + VIRIDIAN_SHOP_COUNT * 2);
    text_draw_str(3, cancel_row, "CANCEL");

    u8 cursor_row;
    if (s_shop_cursor < VIRIDIAN_SHOP_COUNT)
        cursor_row = (u8)(5 + s_shop_cursor * 2);
    else
        cursor_row = cancel_row;
    text_draw_char(1, cursor_row, '>');
}

static void shop_update(void) {
    switch (s_shop_state) {
    case SHOP_IDLE:
        break;

    case SHOP_WELCOME:
        if (dialog_update()) {
            s_shop_cursor = 0;
            shop_draw_menu();
            s_shop_state = SHOP_MENU;
        }
        break;

    case SHOP_MENU:
        if (input_pressed(KEY_UP)) {
            s_shop_cursor = s_shop_cursor == 0 ? 1 : 0;
            shop_draw_menu();
            audio_sfx_play(AUDIO_SFX_SELECT);
        } else if (input_pressed(KEY_DOWN)) {
            s_shop_cursor = s_shop_cursor >= 1 ? 0 : 1;
            shop_draw_menu();
            audio_sfx_play(AUDIO_SFX_SELECT);
        } else if (input_pressed(KEY_A)) {
            audio_sfx_play(AUDIO_SFX_CONFIRM);
            if (s_shop_cursor == 0) {
                s_shop_cursor = 0;
                shop_draw_items();
                s_shop_state = SHOP_BUY_SELECT;
            } else {
                dialog_open();
                dialog_set_text("Thank you!\nCome again!");
                s_shop_state = SHOP_GOODBYE;
            }
        } else if (input_pressed(KEY_B)) {
            audio_sfx_play(AUDIO_SFX_PAUSE_CLOSE);
            dialog_open();
            dialog_set_text("Thank you!\nCome again!");
            s_shop_state = SHOP_GOODBYE;
        }
        break;

    case SHOP_BUY_SELECT: {
        u8 total = (u8)(VIRIDIAN_SHOP_COUNT + 1);
        if (input_pressed(KEY_UP)) {
            s_shop_cursor = s_shop_cursor == 0 ? (u8)(total - 1) : (u8)(s_shop_cursor - 1);
            shop_draw_items();
            audio_sfx_play(AUDIO_SFX_SELECT);
        } else if (input_pressed(KEY_DOWN)) {
            s_shop_cursor = s_shop_cursor >= (u8)(total - 1) ? 0 : (u8)(s_shop_cursor + 1);
            shop_draw_items();
            audio_sfx_play(AUDIO_SFX_SELECT);
        } else if (input_pressed(KEY_A)) {
            audio_sfx_play(AUDIO_SFX_CONFIRM);
            if (s_shop_cursor >= VIRIDIAN_SHOP_COUNT) {
                s_shop_cursor = 0;
                shop_draw_menu();
                s_shop_state = SHOP_MENU;
            } else {
                u16 price = s_viridian_shop[s_shop_cursor].price;
                ItemId id = s_viridian_shop[s_shop_cursor].id;
                if (g_player_money < price) {
                    dialog_open();
                    dialog_set_text("You don't have\nenough money.");
                    s_shop_state = SHOP_BUY_DONE;
                } else if (!bag_add(id, 1)) {
                    dialog_open();
                    dialog_set_text("You can't carry\nany more items.");
                    s_shop_state = SHOP_BUY_DONE;
                } else {
                    game_subtract_money(price);
                    dialog_open();
                    static char buy_msg[48];
                    char *p = buy_msg;
                    p[0]='H';p[1]='e';p[2]='r';p[3]='e';p[4]=' ';
                    p[5]='y';p[6]='o';p[7]='u';p[8]=' ';p[9]='a';
                    p[10]='r';p[11]='e';p[12]='!';p[13]='\0';
                    dialog_set_text(buy_msg);
                    s_shop_state = SHOP_BUY_DONE;
                }
            }
        } else if (input_pressed(KEY_B)) {
            audio_sfx_play(AUDIO_SFX_PAUSE_CLOSE);
            s_shop_cursor = 0;
            shop_draw_menu();
            s_shop_state = SHOP_MENU;
        }
        break;
    }

    case SHOP_BUY_DONE:
        if (dialog_update()) {
            s_shop_cursor = 0;
            shop_draw_items();
            s_shop_state = SHOP_BUY_SELECT;
        }
        break;

    case SHOP_GOODBYE:
        if (dialog_update()) {
            s_shop_state = SHOP_IDLE;
            s_blocks_input = FALSE;
            s_active_script_id = 0;
            text_clear();
        }
        break;
    }
}

bool8 script_shop_active(void) {
    return s_shop_state != SHOP_IDLE;
}

bool8 script_viridian_old_man_blocks(s32 x, s32 y) {
    if (!g_world.map || g_world.map->map_id != MAP_VIRIDIAN_CITY ||
        flags_get(FLAG_GOT_POKEDEX))
        return FALSE;

    // Close both approach tiles and the gap beside the sleepy Old Man until
    // the parcel/Pokédex sequence is complete.
    return y == 9 && (x == 18 || x == 19);
}

// Pokered's Pokécenter nurse interaction is a small multi-step script rather
// than ordinary NPC text: welcome, yes/no choice, healing, and farewell.
enum {
    POKECENTER_IDLE = 0,
    POKECENTER_WELCOME,
    POKECENTER_CHOICE,
    POKECENTER_NEED_PARTY,
    POKECENTER_FIT_PARTY,
    POKECENTER_FAREWELL,
    POKECENTER_RELEASE,
};
static u8 s_pokecenter_state = POKECENTER_IDLE;

void script_viridian_city(void) {
    if (!flags_get(FLAG_GOT_POKEDEX) && g_world.npc_count > 6) {
        // Keep the sleeping gate visible and keep the later Old Man hidden.
        g_world.npcs[4].flags &= (u8)~NPCF_HIDDEN;
        g_world.npcs[4].movement = NPC_MOVE_STAY;
        g_world.npcs[6].flags |= NPCF_HIDDEN;
        NpcState *old_man = &g_world.npcs[6];
        // Restore the gate if a save was made while the old movement script
        // was still active in an earlier build.
        old_man->x = 17;
        old_man->y = 5;
        old_man->px = 17 * 16;
        old_man->py = 5 * 16;
        old_man->walking = FALSE;
        old_man->movement = NPC_MOVE_STAY;
    }

    // After the player receives the Pokedex, the sleepy gate is removed and
    // the awake Old Man becomes visible, matching pokered's toggle objects.
    if (flags_get(FLAG_GOT_POKEDEX) && g_world.npc_count > 6) {
        g_world.npcs[4].flags |= NPCF_HIDDEN;
        NpcState *old_man = &g_world.npcs[6];
        old_man->flags &= (u8)~NPCF_HIDDEN;
        old_man->x = 17;
        old_man->y = 5;
        old_man->px = 17 * 16;
        old_man->py = 5 * 16;
        old_man->walking = FALSE;
        old_man->movement = NPC_MOVE_STAY;
    }
}

void script_viridian_mart(void) {
    if (flags_get(FLAG_GOT_OAKS_PARCEL)) return;

    if (s_viridian_mart_state == 0) {
        dialog_open();
        dialog_set_text("Hey! You came from\nPALLET TOWN?");
        s_viridian_mart_state = 1;
    } else if (s_viridian_mart_state == 1) {
        if (dialog_update()) {
            dialog_open();
            dialog_set_text("You know PROF.OAK, right?\fHis order came in.\nWill you take it to him?\f[NAME] got OAK's PARCEL!");
            s_viridian_mart_state = 2;
        }
    } else if (s_viridian_mart_state == 2 && dialog_update()) {
        flags_set(FLAG_GOT_OAKS_PARCEL);
        s_viridian_mart_state = 3;
    }
}

void script_trigger_npc(u16 script_id, u8 npc_index) {
    if (s_blocks_input) return;
    if (dialog_is_open()) return;

    if (g_world.map && npc_index < g_world.npc_count &&
        (g_world.npcs[npc_index].flags & NPCF_ITEM)) {
        NpcState *item = &g_world.npcs[npc_index];
        ItemId item_id = (ItemId)item->item_id;
        if (bag_add(item_id, 1)) {
            if (item->item_flag < FLAG_COUNT)
                flags_set((GameFlag)item->item_flag);
            item->flags |= NPCF_HIDDEN;
            item_text_build("[NAME] found a ", item_get_name(item_id));
            dialog_open();
            dialog_set_text(s_item_pickup_text);
        } else {
            dialog_open();
            dialog_set_text("You can't carry\nany more items.");
        }
        s_active_script_id = 0;
        s_active_npc_index = npc_index;
        s_npc_script_state = 1;
        s_blocks_input = TRUE;
        return;
    }

    if (g_world.map && npc_index < g_world.npc_count &&
        (g_world.npcs[npc_index].flags & NPCF_TRAINER) &&
        !(g_world.npcs[npc_index].flags & NPCF_TRAINER_DEFEATED)) {
        s_active_npc_index = npc_index;
        s_active_trainer_battle = TRUE;
        s_active_script_id = ACTIVE_TRAINER_DIALOG;
        s_npc_script_state = 1;
        s_blocks_input = TRUE;
        const char *text = g_world.npcs[npc_index].trainer_text;
        if (text && text[0]) {
            dialog_open();
            dialog_set_text(text);
        } else {
            start_active_trainer_battle();
        }
        return;
    }

    if (script_id == 22 && flags_get(FLAG_GOT_POKEDEX) &&
        g_world.map && g_world.map->map_id == MAP_VIRIDIAN_MART) {
        s_active_script_id = script_id;
        s_active_npc_index = npc_index;
        s_blocks_input = TRUE;
        dialog_open();
        dialog_set_text("How may I help you?");
        s_shop_state = SHOP_WELCOME;
        return;
    }

    // Viridian's nurse uses pokered's DisplayPokemonCenterDialogue_ flow.
    // The reference sets the blackout/respawn point only after the player
    // accepts healing, so declining does not change the last center.
    if (script_id == 25 && g_world.map &&
        g_world.map->map_id == MAP_VIRIDIAN_POKECENTER) {
        s_active_script_id = script_id;
        s_active_npc_index = npc_index;
        s_pokecenter_state = POKECENTER_WELCOME;
        dialog_open();
        dialog_set_text("Welcome to our\nPOKeMON CENTER!\fWe heal your\nPOKeMON back to\nperfect health!");
        s_blocks_input = TRUE;
        return;
    }

    if (g_world.map && g_world.map->map_id == MAP_OAKS_LAB &&
        npc_index == 1 && flags_get(FLAG_GOT_OAKS_PARCEL) &&
        !flags_get(FLAG_OAK_GOT_PARCEL)) {
        s_active_script_id = 34;
        s_active_npc_index = npc_index;
        dialog_open();
        dialog_set_text("OAK: Ah, my PARCEL!\fThank you, [NAME]!\nThis will help me with my research.");
        s_npc_script_state = 1;
        s_blocks_input = TRUE;
        return;
    }

    // During the starter sequence the rival is moved by the Oak's Lab
    // cutscene. Do not let the normal rival NPC interaction fire based on the
    // player's position while the rival is selecting his Pokémon.
    if (script_id == 1 && g_world.map &&
        g_world.map->map_id == MAP_OAKS_LAB &&
        flags_get(FLAG_OAK_ASKED_TO_CHOOSE_MON) &&
        !flags_get(FLAG_RIVAL_LEFT_OAKS_LAB))
        return;

    // The starter balls are present in the room from the beginning, but they
    // are not interactive until Oak finishes the introduction and gives the
    // player permission to choose. Ignore them before that point so an early
    // A press cannot leave the player stuck in the ball script state.
    if (script_id >= 10 && script_id <= 12 &&
        !flags_get(FLAG_OAK_ASKED_TO_CHOOSE_MON))
        return;

    // Once the player has chosen a starter, pokered no longer opens the
    // selection flow for another ball. The one remaining ball instead gives
    // Oak's "last POKeMON" message; taken balls are hidden on map load and
    // therefore never reach this path.
    if (script_id >= 10 && script_id <= 12 &&
        flags_get(FLAG_GOT_STARTER)) {
        bool8 taken = FALSE;
        if (script_id == 10)
            taken = flags_get(FLAG_OAKSLAB_CHARMANDER_TAKEN);
        else if (script_id == 11)
            taken = flags_get(FLAG_OAKSLAB_SQUIRTLE_TAKEN);
        else
            taken = flags_get(FLAG_OAKSLAB_BULBASAUR_TAKEN);
        if (!taken) {
            // Use the normal NPC-dialog channel, not the Poké Ball script
            // channel, so OAKSLAB_DONE cannot mistake this for a battle.
            s_active_script_id = 0;
            s_active_npc_index = npc_index;
            dialog_open();
            dialog_set_text("That's PROF.OAK's last\nPOKeMON!");
            s_npc_script_state = 1;
            s_blocks_input = TRUE;
        }
        return;
    }

    // Match pokered's post-selection Oak speech after the Rival has left.
    if (g_world.map && g_world.map->map_id == MAP_OAKS_LAB &&
        npc_index == 1 && flags_get(FLAG_RIVAL_LEFT_OAKS_LAB)) {
        // This is a normal NPC conversation. Clear any stale Poké Ball
        // script id left by the starter sequence so npc_script_tick() can
        // advance the dialog and release player input when it closes.
        s_active_script_id = script_id;
        s_active_npc_index = npc_index;
        dialog_open();
        dialog_set_text("OAK: [NAME], raise your young\nPOKeMON by making it fight!");
        s_npc_script_state = 1;
        s_blocks_input = TRUE;
        return;
    }

    // Old Man capture tutorial: before getting the Pokedex he blocks the
    // path north and refuses to move. After Pokedex he offers to demonstrate
    // catching (the actual Old Man battle type is not yet implemented).
    if (script_id == 21 && !flags_get(FLAG_GOT_POKEDEX)) {
        s_active_script_id = script_id;
        s_active_npc_index = npc_index;
        dialog_open();
        dialog_set_text("Ahh, I've had my\ncoffee now and I\nfeel great!");
        s_npc_script_state = 1;
        s_blocks_input = TRUE;
        return;
    }

    s_active_script_id = script_id;
    s_active_npc_index  = npc_index;

    if (script_id >= 10 && script_id <= 12) {
        // Pokéball interaction handled by oaks_lab script
        s_npc_script_state = 1;
        s_blocks_input = TRUE;
        return;
    }

    // PC interaction: opens the full PC menu.
    if (script_id == 36) {
        s_active_script_id = script_id;
        s_active_npc_index = npc_index;
        s_blocks_input = TRUE;
        pc_menu_open();
        return;
    }

    // Route 1's first Youngster gives the one-time Potion sample.
    if (script_id == 37 && g_world.map &&
        g_world.map->map_id == MAP_ROUTE_1) {
        s_active_script_id = script_id;
        s_active_npc_index = npc_index;
        s_route1_youngster_state = 1;
        s_blocks_input = TRUE;
        dialog_open();
        if (flags_get(FLAG_GOT_ROUTE1_POTION))
            dialog_set_text("I hope that POTION\nhelped your POKeMON!");
        else
            dialog_set_text("Here, take this POTION.\nIt will help your POKeMON!");
        return;
    }

    // Daisy gives the Town Map after the player has the Pokédex.
    if (script_id == 5 && g_world.map &&
        g_world.map->map_id == MAP_RIVALS_HOUSE) {
        if (flags_get(FLAG_GOT_TOWN_MAP)) {
            dialog_open();
            dialog_set_text("Use the TOWN MAP\nto find out where\nyou are!");
            s_npc_script_state = 1;
            s_blocks_input = TRUE;
            return;
        } else if (flags_get(FLAG_GOT_POKEDEX)) {
            dialog_open();
            dialog_set_text("Grandpa asked you\nto run an errand?\fHere, this will\nhelp you!\f[NAME] got a\nTOWN MAP!");
            s_npc_script_state = 1;
            s_blocks_input = TRUE;
            return;
        }
    }

    // Simple pokered object-event dialogue can live directly on the map
    // object. Conditional and stateful events continue through script_id.
    if (npc_index < g_world.npc_count && g_world.npcs[npc_index].text) {
        dialog_open();
        dialog_set_text(g_world.npcs[npc_index].text);
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

void script_trainer_battle_complete(bool8 won) {
    if (!s_active_trainer_battle) return;
    if (won && g_world.map && s_active_npc_index < g_world.npc_count) {
        NpcState *trainer = &g_world.npcs[s_active_npc_index];
        if (trainer->trainer_flag != 0 && trainer->trainer_flag < FLAG_COUNT) {
            flags_set((GameFlag)trainer->trainer_flag);
            trainer->flags |= NPCF_TRAINER_DEFEATED;
        }
    }
    s_active_trainer_battle = FALSE;
}

void script_trigger_background(u16 script_id) {
    if (s_blocks_input || dialog_is_open()) return;

    // Route 1's sign is a pokered-style background event rather than an NPC.
    // Additional sign/text IDs can be added as the generated text table and
    // script interpreter are expanded.
    if (script_id == 1) {
        dialog_open();
        dialog_set_text("ROUTE 1\fPALLET TOWN -\nVIRIDIAN CITY");
    } else if (script_id == 2) {
        dialog_open();
        dialog_set_text("ROUTE 2\fVIRIDIAN CITY -\nPEWTER CITY");
    } else if (script_id == 3) {
        dialog_open();
        dialog_set_text("DIGLETT's CAVE\fROUTE 2");
    } else if (script_id == 4) {
        dialog_open();
        dialog_set_text("ROUTE 22\fVIRIDIAN CITY -\nPOKeMON LEAGUE");
    } else {
        return;
    }

    if (script_id >= 1 && script_id <= 4) {
        s_npc_script_state = 1;
        s_active_script_id = 0;
        s_blocks_input = TRUE;
    }
}

void script_trigger_background_event(const BackgroundEvent *event) {
    if (!event) return;
    if (event->text) {
        if (s_blocks_input || dialog_is_open()) return;
        dialog_open();
        dialog_set_text(event->text);
        s_npc_script_state = 1;
        s_active_script_id = 0;
        s_blocks_input = TRUE;
        return;
    }
    script_trigger_background(event->script_id);
}

// Returns TRUE when NPC dialog is done.
static bool8 npc_script_tick(void) {
    if (!s_blocks_input) return TRUE;

    if (s_active_script_id == ACTIVE_TRAINER_DIALOG) {
        if (dialog_update()) {
            s_npc_script_state = 0;
            start_active_trainer_battle();
        }
        return FALSE;
    }

    if (s_active_script_id == 25 &&
        g_world.map && g_world.map->map_id == MAP_VIRIDIAN_POKECENTER) {
        if (s_pokecenter_state == POKECENTER_WELCOME) {
            if (dialog_update()) {
                dialog_yesno_open();
                s_pokecenter_state = POKECENTER_CHOICE;
            }
            return FALSE;
        }
        if (s_pokecenter_state == POKECENTER_CHOICE) {
            u8 choice = dialog_yesno_update();
            if (choice == 0xFF) return FALSE;

            if (choice) {
                // Respawn inside the Pokécenter, two tiles north of the
                // entrance, facing south toward the exit.
                party_set_healing_point(MAP_VIRIDIAN_POKECENTER, 3, 5, DIR_DOWN);
                dialog_open();
                dialog_set_text("OK. We'll need\nyour POKeMON.");
                s_pokecenter_state = POKECENTER_NEED_PARTY;
            } else {
                dialog_open();
                dialog_set_text("We hope to see\nyou again!");
                s_pokecenter_state = POKECENTER_FAREWELL;
            }
            return FALSE;
        }
        if (s_pokecenter_state == POKECENTER_NEED_PARTY) {
            if (dialog_update()) {
                party_heal_all();
                dialog_open();
                dialog_set_text("Thank you!\nYour POKeMON are\nfighting fit!");
                s_pokecenter_state = POKECENTER_FIT_PARTY;
            }
            return FALSE;
        }
        if (s_pokecenter_state == POKECENTER_FIT_PARTY) {
            if (dialog_update()) {
                dialog_open();
                dialog_set_text("We hope to see\nyou again!");
                s_pokecenter_state = POKECENTER_FAREWELL;
            }
            return FALSE;
        }
        if (s_pokecenter_state == POKECENTER_FAREWELL) {
            if (dialog_update()) {
                // Keep input blocked for one extra frame. This mirrors the
                // generic dialog release behavior and prevents the A press
                // that closed the farewell page from retriggering the nurse.
                s_pokecenter_state = POKECENTER_RELEASE;
            }
            return FALSE;
        }
        if (s_pokecenter_state == POKECENTER_RELEASE) {
            s_pokecenter_state = POKECENTER_IDLE;
            s_active_script_id = 0;
            s_blocks_input = FALSE;
            return TRUE;
        }
    }

    if (s_active_script_id == 36) {
        if (pc_menu_update()) {
            pc_menu_close();
            s_active_script_id = 0;
            s_blocks_input = FALSE;
            return TRUE;
        }
        return FALSE;
    }

    if (s_active_script_id == 37 &&
        g_world.map && g_world.map->map_id == MAP_ROUTE_1) {
        if (s_route1_youngster_state == 1 && dialog_update()) {
            if (!flags_get(FLAG_GOT_ROUTE1_POTION)) {
                if (bag_add(ITEM_POTION, 1)) {
                    flags_set(FLAG_GOT_ROUTE1_POTION);
                    dialog_open();
                    dialog_set_text("[NAME] got a POTION!");
                    s_route1_youngster_state = 2;
                } else {
                    dialog_open();
                    dialog_set_text("You can't carry any\nmore items.");
                    s_route1_youngster_state = 2;
                }
            } else {
                s_route1_youngster_state = 2;
            }
            // Do not let the button press that closed this page retrigger
            // the NPC in player_update on the same frame.
            return FALSE;
        }
        if (s_route1_youngster_state == 2 && dialog_update()) {
            s_route1_youngster_state = 3;
            return FALSE;
        }
        if (s_route1_youngster_state == 3) {
            s_route1_youngster_state = 0;
            s_active_script_id = 0;
            s_blocks_input = FALSE;
            return TRUE;
        }
        return FALSE;
    }

    if (s_active_script_id >= 10 && s_active_script_id <= 12) {
        // Pokéball: handled by oaks_lab script — it clears s_blocks_input
        return FALSE;
    }

    if (s_npc_script_state == 1) {
        if (dialog_update()) {
            if (s_active_script_id == 34) {
                flags_set(FLAG_OAK_GOT_PARCEL);
                s_oak_pokedex_pending = TRUE;
                s_active_script_id = ACTIVE_MAP_SCRIPT;
                s_npc_script_state = 0;
                return FALSE;
            }
            // Daisy gives Town Map after dialog about the errand finishes.
            if (s_active_script_id == 5 && g_world.map &&
                g_world.map->map_id == MAP_RIVALS_HOUSE &&
                !flags_get(FLAG_GOT_TOWN_MAP) && flags_get(FLAG_GOT_POKEDEX)) {
                flags_set(FLAG_GOT_TOWN_MAP);
                bag_add(ITEM_TOWN_MAP, 1);
                // Hide the Town Map background object on the table.
                if (g_world.npc_count > 1)
                    g_world.npcs[1].flags |= NPCF_HIDDEN;
            }
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
    if (s_shop_state != SHOP_IDLE) {
        shop_update();
        return;
    }
    // Map-owned cutscenes use ids >= ACTIVE_MAP_SCRIPT and are advanced by
    // their map script. Generic NPC dialogs must be serviced globally so
    // indoor maps without a map script work the same way as Pallet Town.
    // Oak's Lab has its own map-script tick, so let that handler service
    // normal NPC dialogs there. Otherwise the same A press can close a box
    // and immediately trigger Oak again through player interaction.
    if (s_blocks_input &&
        !(s_active_script_id >= 10 && s_active_script_id <= 12) &&
        s_active_script_id != ACTIVE_MAP_SCRIPT &&
        (!g_world.map || g_world.map->map_id != MAP_OAKS_LAB))
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
static s16 s_pallet_oak_target_x = 8;
static s16 s_pallet_oak_target_y = 2;

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
        // The reference trigger is the grass row immediately above the
        // playable north-exit row in this port's logical coordinates.
        if (flags_get(FLAG_FOLLOWED_OAK_INTO_LAB)) break;
        // These are the two reference-aligned grass trigger tiles. Do not
        // accept the surrounding columns; their cutscene geometry differs.
        // The visual west/east grass tiles do not map one-to-one to the
        // rendered columns after the camera/tile conversion. Restrict by the
        // reference grass row only; surrounding tree tiles are impassable.
        if (p->tile_y != 0) break;

        // Allow an unfinished scene to be retried after a ROM update or a
        // reset. The completed-follow flag remains the permanent guard.
        flags_set(FLAG_OAK_APPEARED_IN_PALLET);
        // Pokered changes to Oak's personal encounter theme when he stops
        // the player at the Route 1 entrance.
        audio_music_play(AUDIO_MUSIC_MEET_PROF_OAK);
        s_pallet_oak_target_x = p->tile_x <= 8 ? 8 : 10;
        s_pallet_oak_target_y = 2;
        p->move_state = MOVE_STATE_FROZEN;
        s_blocks_input = TRUE;
        s_active_script_id = ACTIVE_MAP_SCRIPT;
        g_world.npcs[0].flags &= (u8)~NPCF_HIDDEN;
        g_world.npcs[0].facing = DIR_UP;
        dialog_open();
        dialog_set_text("OAK: Hey! Wait!          \fDon't go out!");
        s_pallet_state = PT_OAK_FIRST_TEXT;
        break;

    case PT_OAK_FIRST_TEXT:
        if (dialog_update()) {
            s_pallet_state = PT_OAK_WALK_TO_PLAYER;
        }
        break;

    case PT_OAK_WALK_TO_PLAYER: {
        NpcState *oak = &g_world.npcs[0];
        if (++s_pallet_move_watchdog > 300) {
            oak->x = (u8)s_pallet_oak_target_x;
            oak->y = (u8)s_pallet_oak_target_y;
            oak->px = (s16)oak->x * 16;
            oak->py = (s16)oak->y * 16;
            oak->walking = FALSE;
            s_pallet_move_watchdog = 0;
        }
        if (world_npc_is_moving(0)) break;

        // Oak's sprite is taller than one logical tile. Use two logical
        // coordinates so he appears one clear tile below the player.
        // Confront the player on the aligned tile below their preserved
        // starting position. The two-tile offset compensates for Oak's
        // two-tile-tall sprite.
        s16 target_x = s_pallet_oak_target_x;
        s16 target_y = s_pallet_oak_target_y;
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
            s_pallet_player_pending = TRUE;
            s_pallet_lab_flag_set = FALSE;
            s_pallet_move_watchdog = 0;
            s_pallet_state = PT_OAK_LEADS_PLAYER;
        }
        break;

    case PT_OAK_LEADS_PLAYER:
        if (s_pallet_player_pending) {
            // Oak always gets the first movement step. The player rejoins
            // only after Oak has started and completed that step.
            if (world_npc_is_moving(0)) break;
            if (s_pallet_route_step == 0) {
                world_npc_start_step(0, s_pallet_oak_route[0]);
                s_pallet_route_step = 1;
                break;
            }
            if (player_script_start_step_forced(DIR_DOWN)) {
                s_pallet_player_pending = FALSE;
                build_pallet_player_fallback_path();
            }
            break;
        }
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
    OAKSLAB_DONT_GO_AWAY,
    OAKSLAB_POKEDEX_WAIT,     // Pokédex entry shown before the choice prompt
    OAKSLAB_YESNO_WAIT,       // YES/NO menu for chosen pokemon
    OAKSLAB_RECEIVED_STARTER,  // received message and energetic message
    OAKSLAB_NICKNAME_YESNO,   // ask whether to nickname the starter
    OAKSLAB_NICKNAME_SCREEN,  // pokered-style naming screen
    OAKSLAB_RIVAL_MOVE,       // rival walks to the opposite starter ball
    OAKSLAB_CHOSE_STARTER,    // set flag, rival dialog
    OAKSLAB_WAIT_BATTLE_TRIGGER, // player walks to the battle position
    OAKSLAB_RIVAL_APPROACH,   // rival walks next to the player
    OAKSLAB_RIVAL_CHALLENGE,  // rival fights player
    OAKSLAB_POST_BATTLE_WAIT, // pause before Rival's exit line
    OAKSLAB_POST_BATTLE_TEXT, // Rival delivers his complete exit dialogue
    OAKSLAB_POST_BATTLE_EXIT, // Rival walks out of the lab
    OAKSLAB_POKEDEX_GRAMPS,       // "Gramps!" text before rival walks in
    OAKSLAB_POKEDEX_RIVAL_WALKIN, // Rival walks UP from lab entrance to Oak
    OAKSLAB_POKEDEX_RIVAL,
    OAKSLAB_POKEDEX_REQUEST,
    OAKSLAB_POKEDEX_INVENTION,
    OAKSLAB_POKEDEX_GOT,
    OAKSLAB_POKEDEX_DREAM,
    OAKSLAB_POKEDEX_RIVAL_LEAVE,
    OAKSLAB_POKEDEX_RIVAL_EXIT,   // Rival walks out of lab after Pokedex scene
    OAKSLAB_DONE,
} OaksLabScriptState;

static OaksLabScriptState s_oakslab_state = OAKSLAB_IDLE;
static u16 s_chosen_ball = 0; // script_id of selected pokeball (10/11/12)

PokemonId script_get_starter_species(void) {
    if (flags_get(FLAG_STARTER_CHARMANDER)) return MON_CHARMANDER;
    if (flags_get(FLAG_STARTER_SQUIRTLE)) return MON_SQUIRTLE;
    if (flags_get(FLAG_STARTER_BULBASAUR)) return MON_BULBASAUR;
    // Compatibility with saves created before the dedicated starter flags
    // were added. The selected lab ball is also persisted as a taken flag.
    if (flags_get(FLAG_OAKSLAB_CHARMANDER_TAKEN)) return MON_CHARMANDER;
    if (flags_get(FLAG_OAKSLAB_SQUIRTLE_TAKEN)) return MON_SQUIRTLE;
    if (flags_get(FLAG_OAKSLAB_BULBASAUR_TAKEN)) return MON_BULBASAUR;
    if (s_chosen_ball == 10) return MON_CHARMANDER;
    if (s_chosen_ball == 11) return MON_SQUIRTLE;
    return MON_BULBASAUR;
}
static u8 s_oakslab_oak_step = 0;
static u8 s_oakslab_player_step = 0;
static char s_starter_nickname[8];
static u8 s_rival_target_npc = 0;
static bool8 s_oakslab_battle_row_armed = FALSE;
static u8 s_oakslab_post_battle_step = 0;
static bool8 s_oakslab_post_battle_started = FALSE;

static void oaks_lab_resume_after_starter_save(void) {
    PartyPokemon *starter = party_get_active();

    // Script runtime is rebuilt after loading a save. Persistent flags tell
    // us that the starter sequence reached its handoff to the rival, so
    // reconstruct the fields that would normally be populated in memory.
    if (flags_get(FLAG_STARTER_CHARMANDER)) {
        s_chosen_ball = 10;
        s_rival_target_npc = 4;
    } else if (flags_get(FLAG_STARTER_SQUIRTLE)) {
        s_chosen_ball = 11;
        s_rival_target_npc = 3;
    } else {
        s_chosen_ball = 12;
        s_rival_target_npc = 2;
    }

    if (starter) {
        for (u8 i = 0; i < sizeof(s_starter_nickname) - 1; i++) {
            s_starter_nickname[i] = starter->nickname[i];
            if (!starter->nickname[i]) break;
        }
        s_starter_nickname[sizeof(s_starter_nickname) - 1] = '\0';
    }

    // The selected starter and the rival's selected ball were already taken
    // before the save became available. Restore the rival's staging position,
    // then make the player re-enter the battle row instead of starting the
    // fight merely because the map was reloaded.
    g_world.npcs[1].flags &= (u8)~NPCF_HIDDEN;
    g_world.npcs[5].flags |= NPCF_HIDDEN;
    g_world.npcs[s_chosen_ball - 8].flags |= NPCF_HIDDEN;
    g_world.npcs[s_rival_target_npc].flags |= NPCF_HIDDEN;
    g_world.npcs[0].x = g_world.npcs[s_rival_target_npc].x;
    g_world.npcs[0].y = (u8)(g_world.npcs[s_rival_target_npc].y + 1);
    g_world.npcs[0].px = (s16)g_world.npcs[0].x * 16;
    g_world.npcs[0].py = (s16)g_world.npcs[0].y * 16;
    g_world.npcs[0].walking = FALSE;
    g_world.npcs[0].facing = DIR_UP;
    s_oakslab_battle_row_armed = FALSE;
    s_blocks_input = FALSE;
    s_active_script_id = 0;
    s_oakslab_state = OAKSLAB_WAIT_BATTLE_TRIGGER;
}

bool8 script_oaks_lab_blocks_exit(u8 dir) {
    if (!g_world.map || g_world.map->map_id != MAP_OAKS_LAB)
        return FALSE;
    if (s_oakslab_state != OAKSLAB_WAIT_CHOOSE || dir != DIR_DOWN ||
        s_blocks_input || dialog_is_open() || g_world.player.tile_y != 6)
        return FALSE;

    g_world.npcs[0].facing = DIR_DOWN;
    g_world.npcs[1].facing = DIR_DOWN;
    dialog_open();
    dialog_set_text("OAK: Hey! Don't go\naway yet!");
    s_blocks_input = TRUE;
    s_active_script_id = ACTIVE_MAP_SCRIPT;
    s_oakslab_state = OAKSLAB_DONT_GO_AWAY;
    return TRUE;
}

static const char *s_ball_names[] = {
    /* 10 */ "CHARMANDER",
    /* 11 */ "SQUIRTLE",
    /* 12 */ "BULBASAUR",
};

static const char *s_received_starter_text[] = {
    "[NAME] received a\nCHARMANDER!\fThis POKeMON is\nreally energetic!",
    "[NAME] received a\nSQUIRTLE!\fThis POKeMON is\nreally energetic!",
    "[NAME] received a\nBULBASAUR!\fThis POKeMON is\nreally energetic!",
};

// ─── Route 22 rival script ─────────────────────────────────────────────────

typedef enum {
    R22_IDLE = 0,
    R22_RIVAL_WALK,
    R22_RIVAL_DIALOG,
    R22_BATTLE,
    R22_POST_BATTLE_DIALOG,
    R22_POST_BATTLE_DIALOG_2,
    R22_RIVAL_EXIT,
    R22_DONE,
} Route22ScriptState;

static Route22ScriptState s_route22_state = R22_IDLE;
static u8 s_route22_walk_steps = 0;

void script_route_22(void) {
    PlayerState *p = &g_world.player;

    switch (s_route22_state) {
    case R22_IDLE:
        if (!flags_get(FLAG_ROUTE22_RIVAL_WANTS_BATTLE)) break;
        if (flags_get(FLAG_BEAT_ROUTE22_RIVAL)) break;
        if (g_world.npc_count < 1) break;
        // pokered's two Route 22 Rival objects are both at (25, 5).  Arm
        // the encounter as the player reaches that object, rather than at
        // the eastern edge of the map.
        if (p->tile_x < 24 || p->tile_x > 26) break;
        if (p->tile_y < 4 || p->tile_y > 6) break;
        if (p->move_state != MOVE_STATE_IDLE) break;

        p->move_state = MOVE_STATE_FROZEN;
        s_blocks_input = TRUE;
        s_active_script_id = ACTIVE_MAP_SCRIPT;

        g_world.npcs[0].flags &= (u8)~NPCF_HIDDEN;
        g_world.npcs[0].x = 25;
        g_world.npcs[0].y = (u8)p->tile_y;
        g_world.npcs[0].px = (s16)(25 * 16);
        g_world.npcs[0].py = (s16)(p->tile_y * 16);
        // Pokered leaves the rival facing east/right after his approach.
        g_world.npcs[0].facing = DIR_RIGHT;
        g_world.npcs[0].walking = FALSE;

        audio_music_play(AUDIO_MUSIC_MEET_PROF_OAK);
        s_route22_walk_steps = 0;
        s_route22_state = R22_RIVAL_WALK;
        break;

    case R22_RIVAL_WALK:
        if (world_npc_is_moving(0)) break;
        if (g_world.npcs[0].x > p->tile_x + 2) {
            world_npc_start_step(0, DIR_LEFT);
        } else {
            g_world.npcs[0].facing = DIR_RIGHT;
            p->facing = DIR_LEFT;
            dialog_open();
            dialog_set_text(
                "[RIVAL]: Hey!\n[NAME]!\fYou're going to\nPOKeMON LEAGUE?\fForget it! You\nprobably don't\nhave any BADGEs!\fThe guard won't\nlet you through!\fBy the way, did\nyour POKeMON get\nany stronger?");
            s_route22_state = R22_RIVAL_DIALOG;
        }
        break;

    case R22_RIVAL_DIALOG:
        if (dialog_update()) {
            PartyPokemon *lead = party_get_lead();
            const char *nick = (lead && lead->nickname[0]) ? lead->nickname : NULL;
            battle_setup_route22_rival(nick);
            audio_music_play(AUDIO_MUSIC_TRAINER_BATTLE);
            battle_transition_start();
            game_change_state(GAME_STATE_BATTLE);
            s_route22_state = R22_BATTLE;
        }
        break;

    case R22_BATTLE:
        if (g_game.state != GAME_STATE_OVERWORLD) break;
        if (battle_is_blackout()) {
            s_route22_state = R22_DONE;
            // The battle return restores the map, but the player was frozen
            // by the cutscene before entering battle. Release that state on
            // the loss path so the overworld cannot remain locked.
            p->move_state = MOVE_STATE_IDLE;
            s_blocks_input = FALSE;
            s_active_script_id = 0;
            break;
        }
        flags_set(FLAG_BEAT_ROUTE22_RIVAL);
        flags_clear(FLAG_ROUTE22_RIVAL_WANTS_BATTLE);
        p->move_state = MOVE_STATE_FROZEN;
        s_blocks_input = TRUE;
        audio_music_play(AUDIO_MUSIC_MEET_PROF_OAK);
        dialog_open();
        dialog_set_text(
            "[RIVAL]: Awww!\nYou just lucked\nout!");
        s_route22_state = R22_POST_BATTLE_DIALOG;
        break;

    case R22_POST_BATTLE_DIALOG:
        if (dialog_update()) {
            dialog_open();
            dialog_set_text(
                "[RIVAL]: What?\nWhy do I have 2\nPOKeMON?\fYou should catch\nsome more too!");
            s_route22_state = R22_POST_BATTLE_DIALOG_2;
        }
        break;

    case R22_POST_BATTLE_DIALOG_2:
        if (dialog_update()) {
            s_route22_state = R22_RIVAL_EXIT;
            s_route22_walk_steps = 0;
        }
        break;

    case R22_RIVAL_EXIT:
        if (world_npc_is_moving(0)) break;
        if (g_world.npcs[0].x < 39) {
            world_npc_start_step(0, DIR_RIGHT);
        } else {
            g_world.npcs[0].flags |= NPCF_HIDDEN;
            audio_music_play(g_world.map->music_id);
            p->move_state = MOVE_STATE_IDLE;
            s_blocks_input = FALSE;
            s_active_script_id = 0;
            s_route22_state = R22_DONE;
        }
        break;

    case R22_DONE:
        break;
    }
}

void script_reset_runtime(void) {
    s_blocks_input = FALSE;
    s_active_trainer_battle = FALSE;
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
    s_pallet_oak_target_x = 8;
    s_pallet_oak_target_y = 2;
    s_viridian_mart_state = 0;
    s_pokecenter_state = POKECENTER_IDLE;
    s_oakslab_state = OAKSLAB_IDLE;
    s_oak_pokedex_pending = FALSE;
    s_chosen_ball = 0;
    s_oakslab_oak_step = 0;
    s_oakslab_player_step = 0;
    s_starter_nickname[0] = '\0';
    s_rival_target_npc = 0;
    s_oakslab_battle_row_armed = FALSE;
    s_oakslab_post_battle_step = 0;
    s_oakslab_post_battle_started = FALSE;
    s_shop_state = SHOP_IDLE;
    s_shop_cursor = 0;
    s_route22_state = R22_IDLE;
    s_route22_walk_steps = 0;
}

static void oaks_lab_start_rival_choice(void) {
    // pokered's opposite starter mapping:
    // Charmander -> Squirtle, Squirtle -> Bulbasaur, Bulbasaur -> Charmander.
    if (s_chosen_ball == 10) s_rival_target_npc = 4;      // right ball
    else if (s_chosen_ball == 11) s_rival_target_npc = 3; // middle ball
    else s_rival_target_npc = 2;                          // left ball
    if (s_rival_target_npc == 2) flags_set(FLAG_OAKSLAB_CHARMANDER_TAKEN);
    else if (s_rival_target_npc == 3) flags_set(FLAG_OAKSLAB_BULBASAUR_TAKEN);
    else flags_set(FLAG_OAKSLAB_SQUIRTLE_TAKEN);
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
    if (s_oak_pokedex_pending) {
        s_oak_pokedex_pending = FALSE;
        s_blocks_input = TRUE;
        s_active_script_id = ACTIVE_MAP_SCRIPT;
        dialog_open();
        dialog_set_text("[RIVAL]: Gramps!");
        s_oakslab_state = OAKSLAB_POKEDEX_GRAMPS;
    }

    // NPC dialog ticking. Poké Ball ids 10-12 belong to the starter state
    // machine; every other active NPC id is a normal conversation and must
    // be serviced here, including the lab scientists (13 and 14).
    bool8 is_pokeball_script =
        (s_active_script_id >= 10 && s_active_script_id <= 12);
    if (s_blocks_input && !is_pokeball_script &&
        s_active_script_id != ACTIVE_MAP_SCRIPT) {
        npc_script_tick();
        return;
    }

    switch (s_oakslab_state) {
    case OAKSLAB_IDLE:
        if (!flags_get(FLAG_FOLLOWED_OAK_INTO_LAB)) break;
        if (flags_get(FLAG_OAK_ASKED_TO_CHOOSE_MON)) {
            if (flags_get(FLAG_GOT_STARTER) &&
                !flags_get(FLAG_BATTLED_RIVAL_IN_OAKS_LAB)) {
                oaks_lab_resume_after_starter_save();
                break;
            }
            // Re-entry after first time: Oak2 already walked in, fix NPC visibility
            g_world.npcs[5].flags |= NPCF_HIDDEN;
            g_world.npcs[1].flags &= (u8)~NPCF_HIDDEN;
            s_oakslab_state = OAKSLAB_WAIT_CHOOSE;
            break;
        }
        // First-time entry: freeze player, begin Oak2 walk-in animation
        // Oak2 is normally hidden; reveal the temporary reference-position
        // sprite only when the entrance cutscene is actually starting.
        g_world.npcs[5].flags &= (u8)~NPCF_HIDDEN;
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

    case OAKSLAB_DONT_GO_AWAY:
        if (dialog_update()) {
            s_blocks_input = FALSE;
            s_active_script_id = 0;
            s_oakslab_state = OAKSLAB_WAIT_CHOOSE;
        }
        break;

    case OAKSLAB_POKEDEX_GRAMPS:
        if (dialog_update()) {
            NpcState *rival = &g_world.npcs[0];
            rival->flags &= (u8)~NPCF_HIDDEN;
            rival->x = 5;
            rival->y = 11;
            rival->px = 5 * 16;
            rival->py = 11 * 16;
            rival->facing = DIR_UP;
            rival->walking = FALSE;
            s_oakslab_post_battle_step = 0;
            s_oakslab_state = OAKSLAB_POKEDEX_RIVAL_WALKIN;
        }
        break;

    case OAKSLAB_POKEDEX_RIVAL_WALKIN:
        if (world_npc_is_moving(0)) break;
        if (s_oakslab_post_battle_step < 7) {
            world_npc_start_step(0, DIR_UP);
            s_oakslab_post_battle_step++;
            break;
        }
        g_world.npcs[0].facing = DIR_UP;
        g_world.npcs[1].facing = DIR_DOWN;
        dialog_open();
        dialog_set_text("[RIVAL]: What did\nyou call me for?");
        s_oakslab_state = OAKSLAB_POKEDEX_RIVAL;
        break;

    case OAKSLAB_POKEDEX_RIVAL:
        if (dialog_update()) {
            dialog_open();
            dialog_set_text("OAK: Oh right! I\nhave a request\nof you two.");
            s_oakslab_state = OAKSLAB_POKEDEX_REQUEST;
        }
        break;

    case OAKSLAB_POKEDEX_REQUEST:
        if (dialog_update()) {
            dialog_open();
            dialog_set_text("On the desk there\nis my invention,\nPOKeDEX!\fIt automatically\nrecords data on\nPOKeMON you've\nseen or caught!\fIt's a hi-tech\nencyclopedia!");
            s_oakslab_state = OAKSLAB_POKEDEX_INVENTION;
        }
        break;

    case OAKSLAB_POKEDEX_INVENTION:
        if (dialog_update()) {
            dialog_open();
            dialog_set_text("OAK: [NAME] and\n[RIVAL]! Take\nthese with you!\f[NAME] got\nPOKeDEX from OAK!");
            s_oakslab_state = OAKSLAB_POKEDEX_GOT;
        }
        break;

    case OAKSLAB_POKEDEX_GOT:
        if (dialog_update()) {
            dialog_open();
            dialog_set_text("To make a complete\nguide on all the\nPOKeMON in the\nworld...\fThat was my dream!\fBut, I'm too old!\nI can't do it!\fSo, I want you two\nto fulfill my\ndream for me!\fGet moving, you\ntwo!\fThis is a great\nundertaking in\nPOKeMON history!");
            s_oakslab_state = OAKSLAB_POKEDEX_DREAM;
        }
        break;

    case OAKSLAB_POKEDEX_DREAM:
        if (dialog_update()) {
            g_world.npcs[0].facing = DIR_RIGHT;
            dialog_open();
            dialog_set_text("[RIVAL]: Alright\nGramps! Leave it\nall to me!\f[NAME], I hate to\nsay it, but I\ndon't need you!\fI know! I'll\nborrow a TOWN MAP\nfrom my sis!\fI'll tell her not\nto lend you one,\n[NAME]! Hahaha!");
            s_oakslab_state = OAKSLAB_POKEDEX_RIVAL_LEAVE;
        }
        break;

    case OAKSLAB_POKEDEX_RIVAL_LEAVE:
        if (dialog_update()) {
            flags_set(FLAG_GOT_POKEDEX);
            s_oakslab_post_battle_step = 0;
            s_oakslab_state = OAKSLAB_POKEDEX_RIVAL_EXIT;
        }
        break;

    case OAKSLAB_POKEDEX_RIVAL_EXIT: {
        NpcState *rival = &g_world.npcs[0];
        if (world_npc_is_moving(0)) break;
        if (s_oakslab_post_battle_step < 7) {
            world_npc_start_step(0, DIR_DOWN);
            s_oakslab_post_battle_step++;
            break;
        }
        rival->flags |= NPCF_HIDDEN;
        rival->walking = FALSE;
        s_active_script_id = 0;
        s_blocks_input = FALSE;
        g_world.player.move_state = MOVE_STATE_IDLE;
        s_oakslab_state = OAKSLAB_DONE;
        break;
    }

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
        if (s_chosen_ball == 10) flags_set(FLAG_OAKSLAB_CHARMANDER_TAKEN);
        else if (s_chosen_ball == 11) flags_set(FLAG_OAKSLAB_SQUIRTLE_TAKEN);
        else flags_set(FLAG_OAKSLAB_BULBASAUR_TAKEN);
        flags_set(FLAG_GOT_STARTER);
        flags_set(FLAG_ROUTE22_RIVAL_WANTS_BATTLE);
        if (s_chosen_ball == 10) flags_set(FLAG_STARTER_CHARMANDER);
        else if (s_chosen_ball == 11) flags_set(FLAG_STARTER_SQUIRTLE);
        else flags_set(FLAG_STARTER_BULBASAUR);
        party_set_starter(script_get_starter_species(), s_starter_nickname);
        g_world.npcs[s_active_npc_index].flags |= NPCF_HIDDEN;
        s_blocks_input = TRUE;
        dialog_open();
        dialog_set_text(s_received_starter_text[s_chosen_ball - 10]);
        s_oakslab_state = OAKSLAB_RECEIVED_STARTER;
        break;

    case OAKSLAB_RECEIVED_STARTER:
        if (dialog_update()) {
            dialog_open();
            dialog_yesno_open();
            dialog_set_text("Do you want to give a\nnickname to this POKeMON?");
            s_oakslab_state = OAKSLAB_NICKNAME_YESNO;
        }
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
            party_set_active_nickname(s_starter_nickname);
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
        // battle_end() sets this flag before returning to the lab. Run the
        // reference's post-battle Rival exit before restoring player control.
        if (flags_get(FLAG_BATTLED_RIVAL_IN_OAKS_LAB) &&
            !flags_get(FLAG_RIVAL_LEFT_OAKS_LAB) &&
            !s_oakslab_post_battle_started) {
            s_oakslab_post_battle_started = TRUE;
            s_oakslab_post_battle_step = 0;
            s_blocks_input = TRUE;
            s_active_script_id = ACTIVE_MAP_SCRIPT;
            g_world.player.move_state = MOVE_STATE_FROZEN;
            s_oakslab_state = OAKSLAB_POST_BATTLE_WAIT;
            break;
        }
        if (flags_get(FLAG_BATTLED_RIVAL_IN_OAKS_LAB))
            break;
        if (!dialog_is_open()) {
            s_blocks_input = FALSE;
            g_world.player.move_state = MOVE_STATE_IDLE;
            battle_setup_rival(s_chosen_ball, s_starter_nickname);
            game_change_state(GAME_STATE_BATTLE);
        } else {
            dialog_update();
        }
        break;

    case OAKSLAB_POST_BATTLE_WAIT:
        if (s_oakslab_post_battle_step < 20) {
            s_oakslab_post_battle_step++;
            break;
        }
        dialog_open();
        // Matches pokered's OaksLabRivalSmellYouLaterText, including the
        // first page that explains Rival will toughen up his POKeMON and the
        // closing page addressed to the player and Oak.
        dialog_set_text("RIVAL: Okay!\nI'll make my\nPOKeMON fight to\ntoughen it up!\f[NAME]! Gramps!\nSmell you later!");
        s_oakslab_state = OAKSLAB_POST_BATTLE_TEXT;
        break;

    case OAKSLAB_POST_BATTLE_TEXT:
        if (dialog_update()) {
            s_oakslab_post_battle_step = 0;
            s_oakslab_state = OAKSLAB_POST_BATTLE_EXIT;
        }
        break;

    case OAKSLAB_POST_BATTLE_EXIT: {
        NpcState *rival = &g_world.npcs[0];
        if (world_npc_is_moving(0)) break;
        if (s_oakslab_post_battle_step < 5) {
            world_npc_start_step(0, DIR_DOWN);
            s_oakslab_post_battle_step++;
            break;
        }
        rival->flags |= NPCF_HIDDEN;
        rival->walking = FALSE;
        flags_set(FLAG_RIVAL_LEFT_OAKS_LAB);
        s_blocks_input = FALSE;
        g_world.player.move_state = MOVE_STATE_IDLE;
        s_oakslab_post_battle_step = 0;
        s_oakslab_state = OAKSLAB_DONE;
        break;
    }
    }
}
