#include "pc.h"
#include "text.h"
#include "input.h"
#include "audio.h"
#include "dialog.h"
#include "party.h"
#include "item.h"
#include "game.h"
#include "world.h"

__attribute__((section(".ewram"))) PcState g_pc;

void pc_init(void) {
    g_pc.current_box = 0;
    for (u8 b = 0; b < PC_NUM_BOXES; b++) {
        g_pc.boxes[b].count = 0;
        for (u8 i = 0; i < PC_BOX_SIZE; i++)
            g_pc.boxes[b].mons[i].species = 0;
    }
    g_pc.item_count = 0;
    for (u8 i = 0; i < PC_ITEM_SLOTS; i++) {
        g_pc.items[i].id = 0;
        g_pc.items[i].quantity = 0;
    }
}

bool8 pc_box_has_space(void) {
    return g_pc.boxes[g_pc.current_box].count < PC_BOX_SIZE;
}

u8 pc_current_box_count(void) {
    return g_pc.boxes[g_pc.current_box].count;
}

void pc_change_box(u8 box) {
    if (box < PC_NUM_BOXES)
        g_pc.current_box = box;
}

bool8 pc_deposit_pokemon(u8 party_slot) {
    if (party_slot >= g_party.count) return FALSE;
    if (g_party.count <= 1) return FALSE;
    PcBox *box = &g_pc.boxes[g_pc.current_box];
    if (box->count >= PC_BOX_SIZE) return FALSE;

    box->mons[box->count] = g_party.mons[party_slot];
    box->count++;

    // Remove from party by shifting
    for (u8 i = party_slot; (u8)(i + 1) < g_party.count; i++)
        g_party.mons[i] = g_party.mons[i + 1];
    g_party.count--;
    return TRUE;
}

bool8 pc_withdraw_pokemon(u8 box_slot) {
    PcBox *box = &g_pc.boxes[g_pc.current_box];
    if (box_slot >= box->count) return FALSE;
    if (g_party.count >= PARTY_SIZE) return FALSE;

    g_party.mons[g_party.count] = box->mons[box_slot];
    g_party.count++;

    // Remove from box by shifting
    for (u8 i = box_slot; (u8)(i + 1) < box->count; i++)
        box->mons[i] = box->mons[i + 1];
    box->count--;
    return TRUE;
}

bool8 pc_deposit_item(ItemId id, u8 quantity) {
    if (id == ITEM_NONE || id >= ITEM_COUNT || quantity == 0) return FALSE;
    for (u8 i = 0; i < g_pc.item_count; i++) {
        if (g_pc.items[i].id == (u8)id) {
            u16 total = (u16)g_pc.items[i].quantity + quantity;
            if (total > 99) return FALSE;
            g_pc.items[i].quantity = (u8)total;
            return TRUE;
        }
    }
    if (g_pc.item_count >= PC_ITEM_SLOTS) return FALSE;
    g_pc.items[g_pc.item_count].id = (u8)id;
    g_pc.items[g_pc.item_count].quantity = quantity;
    g_pc.item_count++;
    return TRUE;
}

bool8 pc_withdraw_item(ItemId id, u8 quantity) {
    if (id == ITEM_NONE || id >= ITEM_COUNT || quantity == 0) return FALSE;
    for (u8 i = 0; i < g_pc.item_count; i++) {
        if (g_pc.items[i].id == (u8)id) {
            if (g_pc.items[i].quantity < quantity) return FALSE;
            g_pc.items[i].quantity -= quantity;
            if (g_pc.items[i].quantity == 0) {
                for (u8 j = i; (u8)(j + 1) < g_pc.item_count; j++)
                    g_pc.items[j] = g_pc.items[j + 1];
                g_pc.item_count--;
            }
            return bag_add(id, quantity);
        }
    }
    return FALSE;
}

u8 pc_item_count(ItemId id) {
    for (u8 i = 0; i < g_pc.item_count; i++)
        if (g_pc.items[i].id == (u8)id)
            return g_pc.items[i].quantity;
    return 0;
}

void pc_export(PcState *out) { *out = g_pc; }
void pc_import(const PcState *in) { g_pc = *in; }

// ─── PC Menu UI ─────────────────────────────────────────────────────────────

typedef enum {
    PC_MENU_MAIN = 0,
    PC_MENU_BILLS_PC,
    PC_MENU_BILLS_DEPOSIT,
    PC_MENU_BILLS_WITHDRAW,
    PC_MENU_BILLS_CHANGE_BOX,
    PC_MENU_PLAYER_PC,
    PC_MENU_PLAYER_DEPOSIT_ITEM,
    PC_MENU_PLAYER_WITHDRAW_ITEM,
    PC_MENU_MSG_WAIT,
} PcMenuState;

static bool8 s_pc_open;
static PcMenuState s_pc_state;
static u8 s_pc_cursor;
static u8 s_pc_sub_cursor;
static u8 s_pc_item_scroll;
static u8 s_pc_withdraw_page;
static u16 s_pc_saved_dispcnt;
static PcMenuState s_pc_after_msg;

static void pc_draw_box(u8 left, u8 top, u8 right, u8 bottom) {
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

static void pc_msg(const char *msg) {
    dialog_open();
    dialog_set_text(msg);
}

static void pc_draw_main(void) {
    text_clear();
    pc_draw_box(0, 0, 29, 19);
    text_draw_str(2, 1, "Access whose PC?");
    text_draw_str(4, 4, "BILL's PC");
    text_draw_str(4, 6, "[NAME]'s PC");
    text_draw_str(4, 8, "PROF.OAK's PC");
    text_draw_str(4, 10, "LOG OFF");

    u8 row = (u8)(4 + s_pc_cursor * 2);
    text_draw_char(2, row, '>');
}

static void pc_draw_bills(void) {
    text_clear();
    pc_draw_box(0, 0, 29, 19);

    char buf[16];
    char *p = buf;
    p[0] = 'B'; p[1] = 'O'; p[2] = 'X'; p[3] = ' ';
    p += 4;
    u8 bn = (u8)(g_pc.current_box + 1);
    if (bn >= 10) *p++ = (char)('0' + bn / 10);
    *p++ = (char)('0' + bn % 10);
    *p = '\0';
    text_draw_str(2, 1, buf);

    text_draw_str(4, 4, "WITHDRAW");
    text_draw_str(4, 6, "DEPOSIT");
    text_draw_str(4, 8, "CHANGE BOX");
    text_draw_str(4, 10, "SEE YA!");

    u8 row = (u8)(4 + s_pc_sub_cursor * 2);
    text_draw_char(2, row, '>');
}

static void pc_draw_deposit_list(void) {
    text_clear();
    pc_draw_box(0, 0, 29, 19);
    text_draw_str(2, 1, "Deposit which?");

    for (u8 i = 0; i < g_party.count && i < 6; i++) {
        u8 row = (u8)(3 + i * 2);
        text_draw_str(4, row, party_mon_display_name(&g_party.mons[i]));
        char lv[6];
        lv[0] = 'L'; lv[1] = 'v';
        u8 l = g_party.mons[i].level;
        if (l >= 100) { lv[2] = '1'; lv[3] = '0'; lv[4] = '0'; lv[5] = '\0'; }
        else if (l >= 10) { lv[2] = (char)('0' + l / 10); lv[3] = (char)('0' + l % 10); lv[4] = '\0'; }
        else { lv[2] = (char)('0' + l); lv[3] = '\0'; }
        text_draw_str(16, row, lv);
    }
    text_draw_str(4, (u8)(3 + g_party.count * 2), "CANCEL");

    u8 row = (u8)(3 + s_pc_sub_cursor * 2);
    text_draw_char(2, row, '>');
}

static void pc_draw_withdraw_list(void) {
    text_clear();
    pc_draw_box(0, 0, 29, 19);
    text_draw_str(2, 1, "Withdraw which?");

    PcBox *box = &g_pc.boxes[g_pc.current_box];
    u8 page_start = (u8)(s_pc_withdraw_page * 7);
    u8 remaining = box->count > page_start ? (u8)(box->count - page_start) : 0;
    u8 shown = remaining > 7 ? 7 : remaining;
    if (box->count > 7) {
        text_draw_str(21, 1, "P");
        text_draw_char(22, 1, (char)('1' + s_pc_withdraw_page));
        text_draw_str(23, 1, "/3");
    }
    for (u8 i = 0; i < shown; i++) {
        u8 row = (u8)(3 + i * 2);
        text_draw_str(4, row, party_mon_display_name(&box->mons[page_start + i]));
        char lv[6];
        lv[0] = 'L'; lv[1] = 'v';
        u8 l = box->mons[i].level;
        if (l >= 10) { lv[2] = (char)('0' + l / 10); lv[3] = (char)('0' + l % 10); lv[4] = '\0'; }
        else { lv[2] = (char)('0' + l); lv[3] = '\0'; }
        text_draw_str(16, row, lv);
    }
    text_draw_str(4, (u8)(3 + shown * 2), "CANCEL");

    u8 row = (u8)(3 + s_pc_sub_cursor * 2);
    text_draw_char(2, row, '>');
}

static void pc_draw_change_box(void) {
    text_clear();
    pc_draw_box(0, 0, 29, 19);
    text_draw_str(2, 1, "Choose a BOX.");

    for (u8 i = 0; i < PC_NUM_BOXES && i < 8; i++) {
        u8 row = (u8)(3 + i * 2);
        char buf[12];
        char *p = buf;
        p[0] = 'B'; p[1] = 'O'; p[2] = 'X'; p[3] = ' ';
        p += 4;
        u8 bn = (u8)(i + 1);
        if (bn >= 10) *p++ = (char)('0' + bn / 10);
        *p++ = (char)('0' + bn % 10);
        *p++ = ' ';
        u8 cnt = g_pc.boxes[i].count;
        if (cnt >= 10) *p++ = (char)('0' + cnt / 10);
        *p++ = (char)('0' + cnt % 10);
        *p++ = '/'; *p++ = '2'; *p++ = '0';
        *p = '\0';
        text_draw_str(4, row, buf);
        if (i == g_pc.current_box)
            text_draw_char(18, row, '*');
    }

    u8 row = (u8)(3 + s_pc_sub_cursor * 2);
    text_draw_char(2, row, '>');
}

static void pc_draw_player_pc(void) {
    text_clear();
    pc_draw_box(0, 0, 29, 19);
    text_draw_str(2, 1, "[NAME]'s PC");
    text_draw_str(4, 4, "WITHDRAW ITEM");
    text_draw_str(4, 6, "DEPOSIT ITEM");
    text_draw_str(4, 8, "LOG OFF");

    u8 row = (u8)(4 + s_pc_sub_cursor * 2);
    text_draw_char(2, row, '>');
}

static void pc_draw_item_withdraw(void) {
    text_clear();
    pc_draw_box(0, 0, 29, 19);
    text_draw_str(2, 1, "Withdraw item:");

    u8 total = (u8)(g_pc.item_count + 1);
    if (s_pc_item_scroll > 0 && s_pc_item_scroll + 7 > total)
        s_pc_item_scroll = total > 7 ? (u8)(total - 7) : 0;
    u8 shown = (u8)(g_pc.item_count - s_pc_item_scroll);
    if (shown > 7) shown = 7;
    for (u8 i = 0; i < shown; i++) {
        u8 item_index = (u8)(s_pc_item_scroll + i);
        u8 row = (u8)(3 + i * 2);
        text_draw_str(4, row, item_get_name((ItemId)g_pc.items[item_index].id));
        char qty[5];
        qty[0] = 'x';
        u8 q = g_pc.items[item_index].quantity;
        if (q >= 10) { qty[1] = (char)('0' + q / 10); qty[2] = (char)('0' + q % 10); qty[3] = '\0'; }
        else { qty[1] = (char)('0' + q); qty[2] = '\0'; }
        text_draw_str(22, row, qty);
    }
    if (g_pc.item_count >= s_pc_item_scroll &&
        g_pc.item_count < (u8)(s_pc_item_scroll + 7))
        text_draw_str(4, (u8)(3 + (g_pc.item_count - s_pc_item_scroll) * 2), "CANCEL");

    if (s_pc_sub_cursor >= s_pc_item_scroll &&
        s_pc_sub_cursor < (u8)(s_pc_item_scroll + 7)) {
        u8 row = (u8)(3 + (s_pc_sub_cursor - s_pc_item_scroll) * 2);
        text_draw_char(2, row, '>');
    }
}

static void pc_draw_item_deposit(void) {
    text_clear();
    pc_draw_box(0, 0, 29, 19);
    text_draw_str(2, 1, "Deposit item:");

    u8 count = 0;
    for (u8 i = 0; i < g_bag.count && count < 7; i++) {
        if (item_is_key_item((ItemId)g_bag.slots[i].id)) continue;
        u8 row = (u8)(3 + count * 2);
        text_draw_str(4, row, item_get_name((ItemId)g_bag.slots[i].id));
        char qty[5];
        qty[0] = 'x';
        u8 q = g_bag.slots[i].quantity;
        if (q >= 10) { qty[1] = (char)('0' + q / 10); qty[2] = (char)('0' + q % 10); qty[3] = '\0'; }
        else { qty[1] = (char)('0' + q); qty[2] = '\0'; }
        text_draw_str(22, row, qty);
        count++;
    }
    text_draw_str(4, (u8)(3 + count * 2), "CANCEL");

    u8 row = (u8)(3 + s_pc_sub_cursor * 2);
    text_draw_char(2, row, '>');
}

void pc_menu_open(void) {
    s_pc_open = TRUE;
    s_pc_state = PC_MENU_MAIN;
    s_pc_cursor = 0;
    s_pc_sub_cursor = 0;
    s_pc_saved_dispcnt = REG_DISPCNT;
    REG_DISPCNT = (u16)((s_pc_saved_dispcnt & (u16)~(DCNT_BG1 | DCNT_BG2 | DCNT_OBJ)) |
                        DCNT_BG0);
    audio_sfx_play(AUDIO_SFX_CONFIRM);
    pc_draw_main();
}

bool8 pc_menu_is_open(void) { return s_pc_open; }

bool8 pc_menu_update(void) {
    if (!s_pc_open) return TRUE;

    if (s_pc_state == PC_MENU_MSG_WAIT) {
        if (dialog_update()) {
            s_pc_state = s_pc_after_msg;
            switch (s_pc_state) {
            case PC_MENU_BILLS_PC: pc_draw_bills(); break;
            case PC_MENU_BILLS_DEPOSIT: pc_draw_deposit_list(); break;
            case PC_MENU_BILLS_WITHDRAW: pc_draw_withdraw_list(); break;
            case PC_MENU_PLAYER_PC: pc_draw_player_pc(); break;
            case PC_MENU_PLAYER_WITHDRAW_ITEM: pc_draw_item_withdraw(); break;
            case PC_MENU_PLAYER_DEPOSIT_ITEM: pc_draw_item_deposit(); break;
            default: pc_draw_main(); break;
            }
        }
        return FALSE;
    }

    if (s_pc_state == PC_MENU_MAIN) {
        if (input_pressed(KEY_UP) && s_pc_cursor > 0) {
            s_pc_cursor--; pc_draw_main(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_DOWN) && s_pc_cursor < 3) {
            s_pc_cursor++; pc_draw_main(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_A)) {
            audio_sfx_play(AUDIO_SFX_CONFIRM);
            if (s_pc_cursor == 0) {
                s_pc_state = PC_MENU_BILLS_PC;
                s_pc_sub_cursor = 0;
                pc_draw_bills();
            } else if (s_pc_cursor == 1) {
                s_pc_state = PC_MENU_PLAYER_PC;
                s_pc_sub_cursor = 0;
                pc_draw_player_pc();
            } else if (s_pc_cursor == 2) {
                pc_msg("POKeDEX rating:\nNot available yet.");
                s_pc_after_msg = PC_MENU_MAIN;
                s_pc_state = PC_MENU_MSG_WAIT;
            } else {
                return TRUE;
            }
        }
        if (input_pressed(KEY_B)) return TRUE;
        return FALSE;
    }

    if (s_pc_state == PC_MENU_BILLS_PC) {
        if (input_pressed(KEY_UP) && s_pc_sub_cursor > 0) {
            s_pc_sub_cursor--; pc_draw_bills(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_DOWN) && s_pc_sub_cursor < 3) {
            s_pc_sub_cursor++; pc_draw_bills(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_A)) {
            audio_sfx_play(AUDIO_SFX_CONFIRM);
            if (s_pc_sub_cursor == 0) {
                s_pc_state = PC_MENU_BILLS_WITHDRAW;
                s_pc_sub_cursor = 0;
                s_pc_withdraw_page = 0;
                pc_draw_withdraw_list();
            } else if (s_pc_sub_cursor == 1) {
                s_pc_state = PC_MENU_BILLS_DEPOSIT;
                s_pc_sub_cursor = 0;
                pc_draw_deposit_list();
            } else if (s_pc_sub_cursor == 2) {
                s_pc_state = PC_MENU_BILLS_CHANGE_BOX;
                s_pc_sub_cursor = 0;
                pc_draw_change_box();
            } else {
                s_pc_state = PC_MENU_MAIN;
                s_pc_cursor = 0;
                pc_draw_main();
            }
        }
        if (input_pressed(KEY_B)) {
            s_pc_state = PC_MENU_MAIN; s_pc_cursor = 0; pc_draw_main();
        }
        return FALSE;
    }

    if (s_pc_state == PC_MENU_BILLS_DEPOSIT) {
        u8 total = (u8)(g_party.count + 1);
        if (input_pressed(KEY_UP) && s_pc_sub_cursor > 0) {
            s_pc_sub_cursor--; pc_draw_deposit_list(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_DOWN) && s_pc_sub_cursor < (u8)(total - 1)) {
            s_pc_sub_cursor++; pc_draw_deposit_list(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_A)) {
            if (s_pc_sub_cursor >= g_party.count) {
                s_pc_state = PC_MENU_BILLS_PC; s_pc_sub_cursor = 1; pc_draw_bills();
            } else if (g_party.count <= 1) {
                pc_msg("You can't deposit\nyour last POKeMON!");
                s_pc_after_msg = PC_MENU_BILLS_DEPOSIT;
                s_pc_state = PC_MENU_MSG_WAIT;
            } else if (!pc_box_has_space()) {
                pc_msg("The BOX is full!");
                s_pc_after_msg = PC_MENU_BILLS_DEPOSIT;
                s_pc_state = PC_MENU_MSG_WAIT;
            } else {
                pc_deposit_pokemon(s_pc_sub_cursor);
                pc_msg("POKeMON was\nstored in the PC!");
                s_pc_sub_cursor = 0;
                s_pc_after_msg = PC_MENU_BILLS_DEPOSIT;
                s_pc_state = PC_MENU_MSG_WAIT;
                audio_sfx_play(AUDIO_SFX_CONFIRM);
            }
        }
        if (input_pressed(KEY_B)) {
            s_pc_state = PC_MENU_BILLS_PC; s_pc_sub_cursor = 1; pc_draw_bills();
        }
        return FALSE;
    }

    if (s_pc_state == PC_MENU_BILLS_WITHDRAW) {
        PcBox *box = &g_pc.boxes[g_pc.current_box];
        u8 shown = box->count > 7 ? 7 : box->count;
        u8 total = (u8)(shown + 1);
        if (input_pressed(KEY_UP) && s_pc_sub_cursor > 0) {
            s_pc_sub_cursor--; pc_draw_withdraw_list(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_DOWN) && s_pc_sub_cursor < (u8)(total - 1)) {
            s_pc_sub_cursor++; pc_draw_withdraw_list(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_A)) {
            if (s_pc_sub_cursor >= shown) {
                s_pc_state = PC_MENU_BILLS_PC; s_pc_sub_cursor = 0; pc_draw_bills();
            } else if (g_party.count >= PARTY_SIZE) {
                pc_msg("Your party is full!");
                s_pc_after_msg = PC_MENU_BILLS_WITHDRAW;
                s_pc_state = PC_MENU_MSG_WAIT;
            } else {
                pc_withdraw_pokemon((u8)(s_pc_withdraw_page * 7 + s_pc_sub_cursor));
                pc_msg("POKeMON was\nwithdrawn!");
                s_pc_sub_cursor = 0;
                s_pc_after_msg = PC_MENU_BILLS_WITHDRAW;
                s_pc_state = PC_MENU_MSG_WAIT;
                audio_sfx_play(AUDIO_SFX_CONFIRM);
            }
        }
        if (input_pressed(KEY_B)) {
            s_pc_state = PC_MENU_BILLS_PC; s_pc_sub_cursor = 0; pc_draw_bills();
        }
        if (input_pressed(KEY_LEFT) && s_pc_withdraw_page > 0) {
            s_pc_withdraw_page--;
            s_pc_sub_cursor = 0;
            pc_draw_withdraw_list();
        }
        if (input_pressed(KEY_RIGHT) && s_pc_withdraw_page < 2 &&
            s_pc_withdraw_page * 7 < box->count) {
            s_pc_withdraw_page++;
            s_pc_sub_cursor = 0;
            pc_draw_withdraw_list();
        }
        return FALSE;
    }

    if (s_pc_state == PC_MENU_BILLS_CHANGE_BOX) {
        u8 max_shown = PC_NUM_BOXES > 8 ? 8 : PC_NUM_BOXES;
        if (input_pressed(KEY_UP) && s_pc_sub_cursor > 0) {
            s_pc_sub_cursor--; pc_draw_change_box(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_DOWN) && s_pc_sub_cursor < (u8)(max_shown - 1)) {
            s_pc_sub_cursor++; pc_draw_change_box(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_A)) {
            pc_change_box(s_pc_sub_cursor);
            audio_sfx_play(AUDIO_SFX_CONFIRM);
            s_pc_state = PC_MENU_BILLS_PC; s_pc_sub_cursor = 2; pc_draw_bills();
        }
        if (input_pressed(KEY_B)) {
            s_pc_state = PC_MENU_BILLS_PC; s_pc_sub_cursor = 2; pc_draw_bills();
        }
        return FALSE;
    }

    if (s_pc_state == PC_MENU_PLAYER_PC) {
        if (input_pressed(KEY_UP) && s_pc_sub_cursor > 0) {
            s_pc_sub_cursor--; pc_draw_player_pc(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_DOWN) && s_pc_sub_cursor < 2) {
            s_pc_sub_cursor++; pc_draw_player_pc(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_A)) {
            audio_sfx_play(AUDIO_SFX_CONFIRM);
            if (s_pc_sub_cursor == 0) {
                s_pc_state = PC_MENU_PLAYER_WITHDRAW_ITEM;
                s_pc_sub_cursor = 0;
                s_pc_item_scroll = 0;
                pc_draw_item_withdraw();
            } else if (s_pc_sub_cursor == 1) {
                s_pc_state = PC_MENU_PLAYER_DEPOSIT_ITEM;
                s_pc_sub_cursor = 0;
                pc_draw_item_deposit();
            } else {
                s_pc_state = PC_MENU_MAIN; s_pc_cursor = 1; pc_draw_main();
            }
        }
        if (input_pressed(KEY_B)) {
            s_pc_state = PC_MENU_MAIN; s_pc_cursor = 1; pc_draw_main();
        }
        return FALSE;
    }

    if (s_pc_state == PC_MENU_PLAYER_WITHDRAW_ITEM) {
        u8 total = (u8)(g_pc.item_count + 1);
        if (input_pressed(KEY_UP)) {
            s_pc_sub_cursor = s_pc_sub_cursor == 0
                ? (u8)(total - 1) : (u8)(s_pc_sub_cursor - 1);
            if (s_pc_sub_cursor < s_pc_item_scroll)
                s_pc_item_scroll = s_pc_sub_cursor;
            pc_draw_item_withdraw(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_DOWN)) {
            s_pc_sub_cursor = s_pc_sub_cursor >= (u8)(total - 1)
                ? 0 : (u8)(s_pc_sub_cursor + 1);
            if (s_pc_sub_cursor >= (u8)(s_pc_item_scroll + 7))
                s_pc_item_scroll = (u8)(s_pc_sub_cursor - 6);
            pc_draw_item_withdraw(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_A)) {
            if (s_pc_sub_cursor >= g_pc.item_count) {
                s_pc_state = PC_MENU_PLAYER_PC; s_pc_sub_cursor = 0; pc_draw_player_pc();
            } else {
                ItemId id = (ItemId)g_pc.items[s_pc_sub_cursor].id;
                if (pc_withdraw_item(id, 1)) {
                    pc_msg("Withdrew 1\nitem from PC!");
                    s_pc_sub_cursor = 0;
                } else {
                    pc_msg("Your BAG is full!");
                }
                s_pc_after_msg = PC_MENU_PLAYER_WITHDRAW_ITEM;
                s_pc_state = PC_MENU_MSG_WAIT;
            }
        }
        if (input_pressed(KEY_B)) {
            s_pc_state = PC_MENU_PLAYER_PC; s_pc_sub_cursor = 0; pc_draw_player_pc();
        }
        return FALSE;
    }

    if (s_pc_state == PC_MENU_PLAYER_DEPOSIT_ITEM) {
        u8 count = 0;
        for (u8 i = 0; i < g_bag.count; i++)
            if (!item_is_key_item((ItemId)g_bag.slots[i].id)) count++;
        if (count > 7) count = 7;
        u8 total = (u8)(count + 1);

        if (input_pressed(KEY_UP) && s_pc_sub_cursor > 0) {
            s_pc_sub_cursor--; pc_draw_item_deposit(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_DOWN) && s_pc_sub_cursor < (u8)(total - 1)) {
            s_pc_sub_cursor++; pc_draw_item_deposit(); audio_sfx_play(AUDIO_SFX_SELECT);
        }
        if (input_pressed(KEY_A)) {
            if (s_pc_sub_cursor >= count) {
                s_pc_state = PC_MENU_PLAYER_PC; s_pc_sub_cursor = 1; pc_draw_player_pc();
            } else {
                // Find the nth non-key item
                u8 found = 0;
                for (u8 i = 0; i < g_bag.count; i++) {
                    if (item_is_key_item((ItemId)g_bag.slots[i].id)) continue;
                    if (found == s_pc_sub_cursor) {
                        ItemId id = (ItemId)g_bag.slots[i].id;
                        if (pc_deposit_item(id, 1)) {
                            bag_remove(id, 1);
                            pc_msg("Deposited 1\nitem in PC!");
                            s_pc_sub_cursor = 0;
                        } else {
                            pc_msg("PC storage is\nfull!");
                        }
                        s_pc_after_msg = PC_MENU_PLAYER_DEPOSIT_ITEM;
                        s_pc_state = PC_MENU_MSG_WAIT;
                        break;
                    }
                    found++;
                }
            }
        }
        if (input_pressed(KEY_B)) {
            s_pc_state = PC_MENU_PLAYER_PC; s_pc_sub_cursor = 1; pc_draw_player_pc();
        }
        return FALSE;
    }

    return FALSE;
}

void pc_menu_close(void) {
    if (!s_pc_open) return;
    s_pc_open = FALSE;
    text_init();
    tilemap_rebuild();
    tilemap_update_scroll();
    REG_DISPCNT = s_pc_saved_dispcnt;
}
