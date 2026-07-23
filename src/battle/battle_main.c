#include "battle.h"
#include "battle_pokemon.h"
#include "battle_calc.h"
#include "battle_ai.h"
#include "battle_rng.h"
#include "type_effectiveness.h"
#include "gfx_battle_sprites.h"
#include "render.h"
#include "gba.h"
#include "text.h"
#include "dialog.h"
#include "input.h"
#include "game.h"
#include "world.h"
#include "irq.h"
#include "flags.h"

typedef enum {
    BS_INIT = 0,
    BS_INTRO,
    BS_SEND_OUT_ENEMY,
    BS_SEND_OUT_PLAYER,
    BS_TURN_START,
    BS_PLAYER_MENU,
    BS_MOVE_SELECT,
    BS_PARTY_MENU,
    BS_ITEM_MENU,
    BS_ACTION_MSG_WAIT,
    BS_TURN_RESOLVE,
    BS_EXECUTE_MOVE,
    BS_EXECUTE_MOVE_WAIT,
    BS_HP_ANIM,
    BS_EXECUTE_EFFECT,
    BS_EXECUTE_EFFECT_WAIT,
    BS_CHECK_FAINT,
    BS_FAINT_MSG,
    BS_NEXT_ATTACKER,
    BS_VICTORY,
    BS_VICTORY_WAIT,
    BS_EXP,
    BS_EXP_WAIT,
    BS_LEVEL_UP,
    BS_LEVEL_UP_WAIT,
    BS_DEFEAT,
    BS_DEFEAT_WAIT,
    BS_END,
} BattleState;

typedef struct {
    BattlePokemon player_mon;
    BattlePokemon enemy_mon;
    BattleState state;
    u8  menu_cursor;
    u8  move_cursor;
    MoveId player_move;
    MoveId enemy_move;
    bool8 player_goes_first;
    u8  turn_phase;
    u16 anim_timer;
    u16 exp_gained;
    u16 saved_dispcnt;
    bool8 crit_flag;
    u16 last_damage;
    u8 last_effectiveness;
    bool8 last_hit;
    PokemonId player_species;
    PokemonId enemy_species;
    const char *player_nickname;
} BattleCtx;

static BattleCtx s_battle;

static u32 s_player_sprite_buf[BATTLE_SPRITE_WORDS];
static u32 s_enemy_sprite_buf[BATTLE_SPRITE_WORDS];

static char s_msg_buf[128];

static const MoveId s_starter_moves[][2] = {
    [MON_BULBASAUR]  = { MOVE_TACKLE, MOVE_GROWL },
    [MON_CHARMANDER] = { MOVE_SCRATCH, MOVE_GROWL },
    [MON_SQUIRTLE]   = { MOVE_TACKLE, MOVE_TAIL_WHIP },
};

static const char *species_name(PokemonId id) {
    switch (id) {
    case MON_BULBASAUR:  return "BULBASAUR";
    case MON_CHARMANDER: return "CHARMANDER";
    case MON_SQUIRTLE:   return "SQUIRTLE";
    default:             return "POKeMON";
    }
}

static const char *mon_display_name(const BattlePokemon *mon) {
    return mon->nickname ? mon->nickname : species_name(mon->species);
}

static void str_copy(char *dst, const char *src) {
    while (*src) *dst++ = *src++;
    *dst = '\0';
}

static char *str_append(char *dst, const char *src) {
    while (*src) *dst++ = *src++;
    return dst;
}

static char *str_append_num(char *dst, u16 val) {
    char tmp[6];
    u8 len = 0;
    if (val == 0) { *dst++ = '0'; return dst; }
    while (val > 0) { tmp[len++] = (char)('0' + val % 10); val /= 10; }
    for (u8 i = len; i > 0; i--) *dst++ = tmp[i - 1];
    return dst;
}

static void battle_msg(const char *text) {
    dialog_open();
    dialog_set_text(text);
}

static void setup_palettes(void) {
    const struct { PokemonId id; u8 pal; } species_pal[] = {
        { MON_BULBASAUR,  11 },
        { MON_CHARMANDER, 12 },
        { MON_SQUIRTLE,   13 },
    };

    for (u8 slot = 0; slot < 2; slot++) {
        PokemonId sp = (slot == 0) ? s_battle.player_species : s_battle.enemy_species;
        u8 src_pal = 11;
        for (u8 i = 0; i < 3; i++)
            if (species_pal[i].id == sp) { src_pal = species_pal[i].pal; break; }

        vu16 *bg_src = (vu16 *)MEM_PAL + src_pal * 16;
        vu16 *obj_dst = PAL_OBJ + slot * 16;
        obj_dst[0] = 0;
        for (u8 i = 1; i < 16; i++)
            obj_dst[i] = bg_src[i];
    }
}

static void draw_battle_bg(void) {
    REG_BG3CNT = (u16)(BG_CBB(2) | BG_SBB(26) | BG_4BPP | BG_SIZE_256x256 | 3);
    REG_BG3HOFS = 0;
    REG_BG3VOFS = 0;

    vu32 *cbb2 = (vu32 *)(MEM_VRAM + 0x8000);
    u32 floor_tile[8];
    for (u8 i = 0; i < 8; i++) floor_tile[i] = 0x11111111;
    for (u8 i = 0; i < 8; i++) cbb2[i] = floor_tile[i];

    u32 line_tile[8];
    for (u8 i = 0; i < 7; i++) line_tile[i] = 0x11111111;
    line_tile[7] = 0x22222222;
    for (u8 i = 0; i < 8; i++) cbb2[8 + i] = line_tile[i];

    vu16 *pal = PAL_BG + 1 * 16;
    pal[0] = 0;
    // Soft lavender battle field, matching the original Red battle screen.
    pal[1] = RGB15(29, 28, 31);
    pal[2] = RGB15(21, 20, 23);

    vu16 *sbb = (vu16 *)(MEM_VRAM + 26 * 0x800);
    for (u16 i = 0; i < 32 * 32; i++)
        sbb[i] = (u16)(0 | (1 << 12));

    for (u8 col = 0; col < 30; col++)
        sbb[10 * 32 + col] = (u16)(1 | (1 << 12));
}

static void draw_sprites(void) {
    render_clear_sprites();
    RenderCmd cmd;

    cmd.type = RCMD_DRAW_SPRITE_LARGE;
    cmd.id = 0;
    cmd.x = 72 - 32;
    cmd.y = 80 - 32;
    cmd.param = 0;
    render_submit(cmd);

    cmd.id = 64;
    cmd.x = 176 - 32;
    cmd.y = 40 - 32;
    cmd.param = 1;
    render_submit(cmd);
}

static void draw_box(u8 x, u8 y, u8 w, u8 h) {
    text_draw_tile(x, y, BOX_TL);
    text_draw_tile((u8)(x + w - 1), y, BOX_TR);
    text_draw_tile(x, (u8)(y + h - 1), BOX_BL);
    text_draw_tile((u8)(x + w - 1), (u8)(y + h - 1), BOX_BR);
    for (u8 c = 1; c < w - 1; c++) {
        text_draw_tile((u8)(x + c), y, BOX_TE);
        text_draw_tile((u8)(x + c), (u8)(y + h - 1), BOX_BE);
    }
    for (u8 r = 1; r < h - 1; r++) {
        text_draw_tile(x, (u8)(y + r), BOX_LE);
        text_draw_tile((u8)(x + w - 1), (u8)(y + r), BOX_RE);
        for (u8 c = 1; c < w - 1; c++)
            text_draw_tile((u8)(x + c), (u8)(y + r), BOX_FILL);
    }
}

#define HP_BAR_LEN 6
#define HP_GREEN_PAL  7
#define HP_YELLOW_PAL 8
#define HP_RED_PAL    9

static void setup_hp_palettes(void) {
    vu16 *base = PAL_BG;

    base[HP_GREEN_PAL * 16 + 0] = 0;
    base[HP_GREEN_PAL * 16 + 1] = RGB15(5, 25, 5);
    base[HP_GREEN_PAL * 16 + 2] = RGB15(29, 28, 31);

    base[HP_YELLOW_PAL * 16 + 0] = 0;
    base[HP_YELLOW_PAL * 16 + 1] = RGB15(28, 25, 2);
    base[HP_YELLOW_PAL * 16 + 2] = RGB15(29, 28, 31);

    base[HP_RED_PAL * 16 + 0] = 0;
    base[HP_RED_PAL * 16 + 1] = RGB15(28, 4, 4);
    base[HP_RED_PAL * 16 + 2] = RGB15(29, 28, 31);
}

static void setup_battle_ui_palette(void) {
    // Match the pale lavender field used by the original pokered battle UI.
    vu16 *ui = PAL_BG + TEXT_PAL * 16;
    ui[0] = 0;
    ui[1] = RGB15(2, 2, 2);
    ui[2] = RGB15(29, 28, 31);
}

static u8 hp_palette(u16 hp, u16 max_hp) {
    if (max_hp == 0) return HP_RED_PAL;
    u32 pct = (u32)hp * 100 / max_hp;
    if (pct > 50) return HP_GREEN_PAL;
    if (pct > 20) return HP_YELLOW_PAL;
    return HP_RED_PAL;
}

static void draw_hp_bar(u8 col, u8 row, u16 hp, u16 max_hp) {
    // pokered draws a 48-pixel bar and keeps one pixel visible while HP is
    // nonzero. Use six 8-pixel tiles plus a partial final tile so the bar does
    // not appear empty one or two attacks before the Pokémon actually faints.
    u8 pixels = 0;
    if (max_hp > 0 && hp > 0) {
        pixels = (u8)(((u32)hp * (HP_BAR_LEN * 8) + max_hp - 1) / max_hp);
        if (pixels == 0) pixels = 1;
    }
    u8 pal = hp_palette(hp, max_hp);

    for (u8 i = 0; i < HP_BAR_LEN; i++) {
        u8 tile_pixels = pixels > (u8)(i * 8) ? (u8)(pixels - i * 8) : 0;
        if (tile_pixels >= 8)
            text_draw_tile_pal((u8)(col + i), row, HP_BAR_FILL_TILE, pal);
        else if (tile_pixels > 0)
            text_draw_tile_pal((u8)(col + i), row,
                               (u8)(HP_BAR_PARTIAL_TILE_BASE + tile_pixels - 1), pal);
        else
            text_draw_tile_pal((u8)(col + i), row, HP_BAR_EMPTY_TILE, TEXT_PAL);
    }
}

static void draw_enemy_hud(void) {
    const BattlePokemon *e = &s_battle.enemy_mon;
    draw_box(0, 0, 15, 4);
    text_draw_str(1, 1, mon_display_name(e));
    text_draw_str(1, 2, "Lv");
    char lvl[4]; char *p = str_append_num(lvl, e->level); *p = '\0';
    text_draw_str(3, 2, lvl);
    text_draw_str(5, 2, "HP:");
    draw_hp_bar(8, 2, e->current_hp, e->max_hp);
}

static void draw_player_hud(void) {
    const BattlePokemon *pl = &s_battle.player_mon;
    draw_box(13, 8, 17, 6);
    text_draw_str(14, 9, mon_display_name(pl));
    text_draw_str(14, 10, "Lv");
    char lvl[4]; char *p = str_append_num(lvl, pl->level); *p = '\0';
    text_draw_str(16, 10, lvl);
    text_draw_str(19, 10, "HP:");
    draw_hp_bar(22, 10, pl->current_hp, pl->max_hp);

    char hp_text[16];
    p = str_append_num(hp_text, pl->current_hp);
    p = str_append(p, "/");
    p = str_append_num(p, pl->max_hp);
    *p = '\0';
    text_draw_str(20, 12, "        ");
    text_draw_str(20, 12, hp_text);
}

static void draw_action_menu(void) {
    draw_box(0, 14, 15, 6);
    draw_box(15, 14, 15, 6);
    text_draw_str(17, 15, "FIGHT");
    text_draw_str(24, 15, "PKMN");
    text_draw_str(17, 17, "ITEM");
    text_draw_str(24, 17, "RUN");
    text_draw_char(16, 15, ' ');
    text_draw_char(23, 15, ' ');
    text_draw_char(16, 17, ' ');
    text_draw_char(23, 17, ' ');
    switch (s_battle.menu_cursor) {
    case 0: text_draw_char(16, 15, '>'); break;
    case 1: text_draw_char(23, 15, '>'); break;
    case 2: text_draw_char(16, 17, '>'); break;
    default: text_draw_char(23, 17, '>'); break;
    }
}

static void draw_party_menu(void) {
    const BattlePokemon *pl = &s_battle.player_mon;
    draw_box(0, 14, 30, 6);
    text_draw_str(1, 15, "Choose a POKeMON");
    text_draw_char(1, 17, '>');
    text_draw_str(2, 17, mon_display_name(pl));
    text_draw_str(2, 18, "HP");
    text_draw_str(6, 18, "        ");
    char hp_text[16];
    char *p = str_append_num(hp_text, pl->current_hp);
    p = str_append(p, "/");
    p = str_append_num(p, pl->max_hp);
    *p = '\0';
    text_draw_str(6, 18, hp_text);
    text_draw_str(20, 18, "B: CANCEL");
}

static void draw_item_menu(void) {
    draw_box(0, 14, 30, 6);
    text_draw_str(1, 15, "Choose an ITEM");
    text_draw_str(2, 17, "No items");
    text_draw_str(20, 18, "B: CANCEL");
}

static void draw_move_menu(void) {
    const BattlePokemon *pl = &s_battle.player_mon;
    draw_box(0, 14, 15, 5);
    draw_box(15, 14, 14, 5);

    for (u8 i = 0; i < 4; i++) {
        if (pl->moves[i] == MOVE_NONE) continue;
        const MoveData *md = &g_move_data[pl->moves[i]];
        text_draw_str(2, (u8)(15 + i), md->name);
    }

    text_draw_char(1, (u8)(15 + s_battle.move_cursor), '>');
    for (u8 i = 0; i < 4; i++)
        if (i != s_battle.move_cursor)
            text_draw_char(1, (u8)(15 + i), ' ');

    const MoveData *sel = &g_move_data[pl->moves[s_battle.move_cursor]];
    char pp_str[16];
    char *p = str_append(pp_str, "PP ");
    p = str_append_num(p, pl->pp[s_battle.move_cursor]);
    p = str_append(p, "/");
    p = str_append_num(p, sel->pp);
    *p = '\0';
    text_draw_str(16, 15, "            ");
    text_draw_str(16, 15, pp_str);

    const char *type_name = "NORMAL";
    switch (sel->type) {
    case TYPE_FIRE:     type_name = "FIRE"; break;
    case TYPE_WATER:    type_name = "WATER"; break;
    case TYPE_GRASS:    type_name = "GRASS"; break;
    case TYPE_ELECTRIC: type_name = "ELECTRIC"; break;
    case TYPE_ICE:      type_name = "ICE"; break;
    case TYPE_FIGHTING: type_name = "FIGHTING"; break;
    case TYPE_POISON:   type_name = "POISON"; break;
    case TYPE_GROUND:   type_name = "GROUND"; break;
    case TYPE_FLYING:   type_name = "FLYING"; break;
    case TYPE_PSYCHIC:  type_name = "PSYCHIC"; break;
    case TYPE_BUG:      type_name = "BUG"; break;
    case TYPE_ROCK:     type_name = "ROCK"; break;
    case TYPE_GHOST:    type_name = "GHOST"; break;
    case TYPE_DRAGON:   type_name = "DRAGON"; break;
    default: break;
    }
    text_draw_str(16, 17, "            ");
    text_draw_str(16, 17, type_name);
}

static void redraw_huds(void) {
    draw_enemy_hud();
    draw_player_hud();
}

static void draw_turn_prompt(void) {
    text_draw_str(1, 15, "What will");
    text_draw_str(1, 16, mon_display_name(&s_battle.player_mon));
    text_draw_str(1, 17, "do?");
}

static void clear_lower_ui(void) {
    for (u8 r = 14; r < 20; r++)
        for (u8 c = 0; c < 30; c++)
            text_draw_tile(c, r, TEXT_BLANK_TILE);
}

static BattlePokemon *cur_attacker(void) {
    if (s_battle.turn_phase == 0)
        return s_battle.player_goes_first ? &s_battle.player_mon : &s_battle.enemy_mon;
    return s_battle.player_goes_first ? &s_battle.enemy_mon : &s_battle.player_mon;
}

static BattlePokemon *cur_defender(void) {
    if (s_battle.turn_phase == 0)
        return s_battle.player_goes_first ? &s_battle.enemy_mon : &s_battle.player_mon;
    return s_battle.player_goes_first ? &s_battle.player_mon : &s_battle.enemy_mon;
}

static MoveId cur_move(void) {
    BattlePokemon *atk = cur_attacker();
    if (atk == &s_battle.player_mon) return s_battle.player_move;
    return s_battle.enemy_move;
}

static bool8 is_player_attacking(void) {
    return cur_attacker() == &s_battle.player_mon;
}

static void apply_stat_effect(MoveId move, BattlePokemon *target) {
    const MoveData *md = &g_move_data[move];
    s8 *stage = NULL;
    const char *stat_name = NULL;
    s8 delta = 0;

    switch (md->effect) {
    case EFFECT_STAT_ATK_DOWN1:
        stage = &target->stages[STAT_STAGE_ATK]; stat_name = "ATTACK"; delta = -1; break;
    case EFFECT_STAT_DEF_DOWN1:
        stage = &target->stages[STAT_STAGE_DEF]; stat_name = "DEFENSE"; delta = -1; break;
    case EFFECT_STAT_DEF_DOWN2:
        stage = &target->stages[STAT_STAGE_DEF]; stat_name = "DEFENSE"; delta = -2; break;
    case EFFECT_STAT_SPC_DOWN:
        stage = &target->stages[STAT_STAGE_SPC]; stat_name = "SPECIAL"; delta = -1; break;
    case EFFECT_STAT_ACC_DOWN1:
        stage = &target->stages[STAT_STAGE_ACC]; stat_name = "ACCURACY"; delta = -1; break;
    case EFFECT_STAT_ATK_UP1:
        stage = &target->stages[STAT_STAGE_ATK]; stat_name = "ATTACK"; delta = 1; break;
    case EFFECT_STAT_ATK_UP2:
        stage = &target->stages[STAT_STAGE_ATK]; stat_name = "ATTACK"; delta = 2; break;
    case EFFECT_STAT_DEF_UP1:
        stage = &target->stages[STAT_STAGE_DEF]; stat_name = "DEFENSE"; delta = 1; break;
    case EFFECT_STAT_DEF_UP2:
        stage = &target->stages[STAT_STAGE_DEF]; stat_name = "DEFENSE"; delta = 2; break;
    case EFFECT_STAT_SPD_UP2:
        stage = &target->stages[STAT_STAGE_SPD]; stat_name = "SPEED"; delta = 2; break;
    case EFFECT_STAT_SPC_UP1:
        stage = &target->stages[STAT_STAGE_SPC]; stat_name = "SPECIAL"; delta = 1; break;
    case EFFECT_STAT_SPC_UP2:
        stage = &target->stages[STAT_STAGE_SPC]; stat_name = "SPECIAL"; delta = 2; break;
    case EFFECT_STAT_EVA_UP1:
        stage = &target->stages[STAT_STAGE_EVA]; stat_name = "EVASION"; delta = 1; break;
    default: return;
    }

    if (!stage) return;

    bool8 at_limit = (delta < 0 && *stage <= -6) || (delta > 0 && *stage >= 6);
    s8 new_val = *stage + delta;
    if (new_val < -6) new_val = -6;
    if (new_val > 6) new_val = 6;
    *stage = new_val;

    bool8 is_foe = (target == &s_battle.enemy_mon);
    char *p = s_msg_buf;
    if (is_foe) p = str_append(p, "FOE's ");
    p = str_append(p, mon_display_name(target));
    p = str_append(p, "'s\n");
    p = str_append(p, stat_name);
    if (at_limit)
        p = str_append(p, delta < 0 ? " won't go\nany lower!" : " won't go\nany higher!");
    else
        p = str_append(p, delta < 0 ? " fell!" : " rose!");
    *p = '\0';
    battle_msg(s_msg_buf);
}

void battle_setup_rival(u16 chosen_ball, const char *player_nickname) {
    switch (chosen_ball) {
    case 10:
        s_battle.player_species = MON_CHARMANDER;
        s_battle.enemy_species = MON_SQUIRTLE;
        break;
    case 11:
        s_battle.player_species = MON_SQUIRTLE;
        s_battle.enemy_species = MON_BULBASAUR;
        break;
    default:
        s_battle.player_species = MON_BULBASAUR;
        s_battle.enemy_species = MON_CHARMANDER;
        break;
    }

    s_battle.player_nickname = player_nickname;
}

void battle_init(void) {
    battle_rng_seed(g_vblank_count);

    s_battle.saved_dispcnt = REG_DISPCNT;
    s_battle.state = BS_INIT;
    s_battle.menu_cursor = 0;
    s_battle.move_cursor = 0;
    s_battle.turn_phase = 0;
    s_battle.anim_timer = 0;
    s_battle.exp_gained = 0;
    s_battle.crit_flag = FALSE;
    s_battle.last_damage = 0;
    s_battle.last_effectiveness = TYPE_MUL_NEUTRAL;
    s_battle.last_hit = FALSE;

    u16 player_dv = (u16)(battle_random() | 0x0001);
    u16 rival_dv = 0x9888;

    battle_pokemon_init(&s_battle.player_mon, s_battle.player_species, 5,
                        player_dv, s_starter_moves[s_battle.player_species], 2);
    battle_pokemon_init(&s_battle.enemy_mon, s_battle.enemy_species, 5,
                        rival_dv, s_starter_moves[s_battle.enemy_species], 2);
    s_battle.player_mon.nickname = s_battle.player_nickname &&
        s_battle.player_nickname[0] ? s_battle.player_nickname : NULL;

    text_clear();

    battle_sprite_load_back(s_battle.player_species, s_player_sprite_buf);
    battle_sprite_load_front(s_battle.enemy_species, s_enemy_sprite_buf);

    vu32 *obj_vram = (vu32 *)(MEM_VRAM + 0x10000);
    for (u32 i = 0; i < BATTLE_SPRITE_WORDS; i++)
        obj_vram[i] = s_player_sprite_buf[i];
    for (u32 i = 0; i < BATTLE_SPRITE_WORDS; i++)
        obj_vram[BATTLE_SPRITE_WORDS + i] = s_enemy_sprite_buf[i];

    setup_palettes();
    setup_hp_palettes();
    setup_battle_ui_palette();
    draw_battle_bg();

    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_MAP_1D | DCNT_BG0 | DCNT_BG3 | DCNT_OBJ;

    s_battle.state = BS_INTRO;
    char *p = str_append(s_msg_buf, "[RIVAL] wants to fight!");
    *p = '\0';
    battle_msg(s_msg_buf);
}

void battle_update(void) {
    draw_sprites();

    switch (s_battle.state) {
    case BS_INIT:
        break;

    case BS_INTRO:
        if (dialog_update()) {
            s_battle.state = BS_SEND_OUT_ENEMY;
            char *p = str_append(s_msg_buf, "[RIVAL] sent out\n");
            p = str_append(p, species_name(s_battle.enemy_species));
            p = str_append(p, "!");
            *p = '\0';
            battle_msg(s_msg_buf);
        }
        break;

    case BS_SEND_OUT_ENEMY:
        if (dialog_update()) {
            redraw_huds();
            s_battle.state = BS_SEND_OUT_PLAYER;
            char *p = str_append(s_msg_buf, "Go! ");
            p = str_append(p, mon_display_name(&s_battle.player_mon));
            p = str_append(p, "!");
            *p = '\0';
            battle_msg(s_msg_buf);
        }
        break;

    case BS_SEND_OUT_PLAYER:
        if (dialog_update()) {
            s_battle.state = BS_TURN_START;
        }
        break;

    case BS_TURN_START:
        redraw_huds();
        clear_lower_ui();
        s_battle.state = BS_PLAYER_MENU;
        s_battle.menu_cursor = 0;

        draw_action_menu();
        draw_turn_prompt();
        break;

    case BS_PLAYER_MENU:
        if (input_pressed(KEY_LEFT)) {
            if ((s_battle.menu_cursor & 1) != 0) s_battle.menu_cursor--;
            draw_action_menu();
        }
        if (input_pressed(KEY_RIGHT)) {
            if ((s_battle.menu_cursor & 1) == 0) s_battle.menu_cursor++;
            draw_action_menu();
        }
        if (input_pressed(KEY_UP)) {
            if (s_battle.menu_cursor >= 2) s_battle.menu_cursor -= 2;
            draw_action_menu();
        }
        if (input_pressed(KEY_DOWN)) {
            if (s_battle.menu_cursor < 2) s_battle.menu_cursor += 2;
            draw_action_menu();
        }
        if (input_pressed(KEY_A)) {
            if (s_battle.menu_cursor == 0) {
                s_battle.move_cursor = 0;
                s_battle.state = BS_MOVE_SELECT;
                clear_lower_ui();
                draw_move_menu();
            } else if (s_battle.menu_cursor == 1) {
                s_battle.state = BS_PARTY_MENU;
                clear_lower_ui();
                draw_party_menu();
            } else if (s_battle.menu_cursor == 2) {
                s_battle.state = BS_ITEM_MENU;
                clear_lower_ui();
                draw_item_menu();
            } else {
                clear_lower_ui();
                battle_msg("Can't escape!");
                s_battle.state = BS_ACTION_MSG_WAIT;
            }
        }
        break;

    case BS_PARTY_MENU:
        if (input_pressed(KEY_B)) {
            clear_lower_ui();
            s_battle.state = BS_PLAYER_MENU;
            draw_action_menu();
            draw_turn_prompt();
        } else if (input_pressed(KEY_A)) {
            clear_lower_ui();
            battle_msg("No other POKeMON!");
            s_battle.state = BS_ACTION_MSG_WAIT;
        }
        break;

    case BS_ITEM_MENU:
        if (input_pressed(KEY_B)) {
            clear_lower_ui();
            s_battle.state = BS_PLAYER_MENU;
            draw_action_menu();
            draw_turn_prompt();
        } else if (input_pressed(KEY_A)) {
            clear_lower_ui();
            battle_msg("No items!");
            s_battle.state = BS_ACTION_MSG_WAIT;
        }
        break;

    case BS_ACTION_MSG_WAIT:
        if (dialog_update()) {
            clear_lower_ui();
            s_battle.state = BS_PLAYER_MENU;
            draw_action_menu();
            draw_turn_prompt();
        }
        break;

    case BS_MOVE_SELECT: {
        u8 move_count = 0;
        for (u8 i = 0; i < 4; i++)
            if (s_battle.player_mon.moves[i] != MOVE_NONE) move_count++;

        if (input_pressed(KEY_UP) && s_battle.move_cursor > 0) {
            s_battle.move_cursor--;
            draw_move_menu();
        }
        if (input_pressed(KEY_DOWN) && s_battle.move_cursor < move_count - 1) {
            s_battle.move_cursor++;
            draw_move_menu();
        }
        if (input_pressed(KEY_A)) {
            if (s_battle.player_mon.pp[s_battle.move_cursor] == 0) {
                break;
            }
            s_battle.player_move = s_battle.player_mon.moves[s_battle.move_cursor];
            s_battle.enemy_move = battle_ai_choose_move(&s_battle.enemy_mon);
            s_battle.state = BS_TURN_RESOLVE;
        }
        if (input_pressed(KEY_B)) {
            clear_lower_ui();
            s_battle.menu_cursor = 0;
            s_battle.state = BS_PLAYER_MENU;
            draw_action_menu();
            draw_turn_prompt();
        }
        break;
    }

    case BS_TURN_RESOLVE: {
        u16 p_spd = battle_stat_with_stage(s_battle.player_mon.speed,
                                           s_battle.player_mon.stages[STAT_STAGE_SPD]);
        u16 e_spd = battle_stat_with_stage(s_battle.enemy_mon.speed,
                                           s_battle.enemy_mon.stages[STAT_STAGE_SPD]);
        if (p_spd > e_spd)
            s_battle.player_goes_first = TRUE;
        else if (e_spd > p_spd)
            s_battle.player_goes_first = FALSE;
        else
            s_battle.player_goes_first = (battle_random() & 1) ? TRUE : FALSE;

        s_battle.turn_phase = 0;
        s_battle.state = BS_EXECUTE_MOVE;
        clear_lower_ui();

        BattlePokemon *atk = cur_attacker();
        MoveId move = cur_move();
        const MoveData *md = &g_move_data[move];

        bool8 is_foe = (atk == &s_battle.enemy_mon);
        char *p = s_msg_buf;
        if (is_foe) p = str_append(p, "FOE's ");
        p = str_append(p, mon_display_name(atk));
        p = str_append(p, "\nused ");
        p = str_append(p, md->name);
        p = str_append(p, "!");
        *p = '\0';
        battle_msg(s_msg_buf);

        for (u8 i = 0; i < 4; i++) {
            if (atk->moves[i] == move && atk->pp[i] > 0) {
                atk->pp[i]--;
                break;
            }
        }

        s_battle.last_hit = battle_check_hit(atk, cur_defender(), move);
        if (s_battle.last_hit && md->power > 0) {
            s_battle.crit_flag = battle_check_critical(atk, move);
            s_battle.last_damage = battle_calc_damage(atk, cur_defender(), move, s_battle.crit_flag);
            const PokemonBaseStats *def_base = &g_pokemon_base_stats[cur_defender()->species];
            s_battle.last_effectiveness = type_effectiveness(md->type, def_base->type1, def_base->type2);
        } else {
            s_battle.crit_flag = FALSE;
            s_battle.last_damage = 0;
            s_battle.last_effectiveness = TYPE_MUL_NEUTRAL;
        }
        break;
    }

    case BS_EXECUTE_MOVE:
        if (dialog_update()) {
            if (!s_battle.last_hit) {
                clear_lower_ui();
                battle_msg("Attack missed!");
                s_battle.state = BS_CHECK_FAINT;
                break;
            }
            MoveId move = cur_move();
            const MoveData *md = &g_move_data[move];
            if (md->power > 0 && s_battle.last_damage > 0) {
                BattlePokemon *def = cur_defender();
                if (s_battle.last_damage >= def->current_hp)
                    def->current_hp = 0;
                else
                    def->current_hp -= s_battle.last_damage;
                s_battle.anim_timer = 0;
                s_battle.state = BS_HP_ANIM;
            } else {
                s_battle.state = BS_EXECUTE_EFFECT;
            }
        }
        break;

    case BS_HP_ANIM:
        s_battle.anim_timer++;
        redraw_huds();
        if (s_battle.anim_timer >= 16) {
            s_battle.state = BS_EXECUTE_MOVE_WAIT;
            clear_lower_ui();
            if (s_battle.crit_flag) {
                battle_msg("Critical hit!");
            } else if (s_battle.last_effectiveness > TYPE_MUL_NEUTRAL) {
                battle_msg("It's super effective!");
            } else if (s_battle.last_effectiveness < TYPE_MUL_NEUTRAL &&
                       s_battle.last_effectiveness > TYPE_MUL_NO_EFFECT) {
                battle_msg("It's not very\neffective...");
            } else {
                s_battle.state = BS_EXECUTE_EFFECT;
            }
        }
        break;

    case BS_EXECUTE_MOVE_WAIT:
        if (dialog_update()) {
            if (s_battle.crit_flag && s_battle.last_effectiveness != TYPE_MUL_NEUTRAL) {
                s_battle.crit_flag = FALSE;
                clear_lower_ui();
                if (s_battle.last_effectiveness > TYPE_MUL_NEUTRAL)
                    battle_msg("It's super effective!");
                else if (s_battle.last_effectiveness > TYPE_MUL_NO_EFFECT)
                    battle_msg("It's not very\neffective...");
                else
                    s_battle.state = BS_EXECUTE_EFFECT;
            } else {
                s_battle.state = BS_EXECUTE_EFFECT;
            }
        }
        break;

    case BS_EXECUTE_EFFECT: {
        MoveId move = cur_move();
        const MoveData *md = &g_move_data[move];
        bool8 has_stat_effect = FALSE;
        switch (md->effect) {
        case EFFECT_STAT_ATK_DOWN1: case EFFECT_STAT_DEF_DOWN1:
        case EFFECT_STAT_DEF_DOWN2: case EFFECT_STAT_SPC_DOWN:
        case EFFECT_STAT_ACC_DOWN1:
            apply_stat_effect(move, cur_defender());
            has_stat_effect = TRUE;
            break;
        case EFFECT_STAT_ATK_UP1: case EFFECT_STAT_ATK_UP2:
        case EFFECT_STAT_DEF_UP1: case EFFECT_STAT_DEF_UP2:
        case EFFECT_STAT_SPD_UP2: case EFFECT_STAT_SPC_UP1:
        case EFFECT_STAT_SPC_UP2: case EFFECT_STAT_EVA_UP1:
            apply_stat_effect(move, cur_attacker());
            has_stat_effect = TRUE;
            break;
        default: break;
        }
        s_battle.state = has_stat_effect ? BS_EXECUTE_EFFECT_WAIT : BS_CHECK_FAINT;
        break;
    }

    case BS_EXECUTE_EFFECT_WAIT:
        if (dialog_update()) {
            s_battle.state = BS_CHECK_FAINT;
        }
        break;

    case BS_CHECK_FAINT:
        if (dialog_is_open()) {
            if (dialog_update()) {}
            break;
        }
        redraw_huds();
        if (s_battle.enemy_mon.current_hp == 0) {
            clear_lower_ui();
            char *p = str_append(s_msg_buf, "FOE's ");
            p = str_append(p, species_name(s_battle.enemy_species));
            p = str_append(p, "\nfainted!");
            *p = '\0';
            battle_msg(s_msg_buf);
            s_battle.state = BS_FAINT_MSG;
        } else if (s_battle.player_mon.current_hp == 0) {
            clear_lower_ui();
            char *p = str_append(s_msg_buf, mon_display_name(&s_battle.player_mon));
            p = str_append(p, "\nfainted!");
            *p = '\0';
            battle_msg(s_msg_buf);
            s_battle.state = BS_DEFEAT;
        } else {
            s_battle.state = BS_NEXT_ATTACKER;
        }
        break;

    case BS_FAINT_MSG:
        if (dialog_update()) {
            s_battle.state = BS_VICTORY;
            clear_lower_ui();
            char *p = str_append(s_msg_buf, "[NAME] defeated\n[RIVAL]!");
            *p = '\0';
            battle_msg(s_msg_buf);
        }
        break;

    case BS_NEXT_ATTACKER:
        if (s_battle.turn_phase == 0) {
            s_battle.turn_phase = 1;
            s_battle.state = BS_EXECUTE_MOVE;
            clear_lower_ui();

            BattlePokemon *atk = cur_attacker();
            MoveId move = cur_move();
            const MoveData *md = &g_move_data[move];

            bool8 is_foe = (atk == &s_battle.enemy_mon);
            char *p = s_msg_buf;
            if (is_foe) p = str_append(p, "FOE's ");
            p = str_append(p, mon_display_name(atk));
            p = str_append(p, "\nused ");
            p = str_append(p, md->name);
            p = str_append(p, "!");
            *p = '\0';
            battle_msg(s_msg_buf);

            for (u8 i = 0; i < 4; i++) {
                if (atk->moves[i] == move && atk->pp[i] > 0) {
                    atk->pp[i]--;
                    break;
                }
            }

            s_battle.last_hit = battle_check_hit(atk, cur_defender(), move);
            if (s_battle.last_hit && md->power > 0) {
                s_battle.crit_flag = battle_check_critical(atk, move);
                s_battle.last_damage = battle_calc_damage(atk, cur_defender(), move, s_battle.crit_flag);
                const PokemonBaseStats *def_base = &g_pokemon_base_stats[cur_defender()->species];
                s_battle.last_effectiveness = type_effectiveness(md->type, def_base->type1, def_base->type2);
            } else {
                s_battle.crit_flag = FALSE;
                s_battle.last_damage = 0;
                s_battle.last_effectiveness = TYPE_MUL_NEUTRAL;
            }
        } else {
            s_battle.state = BS_TURN_START;
        }
        break;

    case BS_VICTORY:
        if (dialog_update()) {
            s_battle.state = BS_EXP;
            const PokemonBaseStats *ebase = &g_pokemon_base_stats[s_battle.enemy_species];
            s_battle.exp_gained = (u16)((u32)ebase->base_exp * s_battle.enemy_mon.level / 7 * 3 / 2);
            if (s_battle.exp_gained == 0) s_battle.exp_gained = 1;

            clear_lower_ui();
            char *p = str_append(s_msg_buf, mon_display_name(&s_battle.player_mon));
            p = str_append(p, " gained\n");
            p = str_append_num(p, s_battle.exp_gained);
            p = str_append(p, " EXP. Points!");
            *p = '\0';
            battle_msg(s_msg_buf);
        }
        break;

    case BS_EXP:
        if (dialog_update()) {
            u16 exp_for_6 = 179;
            u16 exp_for_5 = 135;
            u16 current_exp = exp_for_5 + s_battle.exp_gained;
            if (current_exp >= exp_for_6) {
                s_battle.player_mon.level = 6;
                const PokemonBaseStats *pbase = &g_pokemon_base_stats[s_battle.player_species];
                u16 old_max = s_battle.player_mon.max_hp;
                s_battle.player_mon.max_hp = (u16)(((u32)(pbase->hp + dv_hp(s_battle.player_mon.dv)) * 2 * 6) / 100 + 6 + 10);
                s_battle.player_mon.attack = (u16)(((u32)(pbase->attack + dv_attack(s_battle.player_mon.dv)) * 2 * 6) / 100 + 5);
                s_battle.player_mon.defense = (u16)(((u32)(pbase->defense + dv_defense(s_battle.player_mon.dv)) * 2 * 6) / 100 + 5);
                s_battle.player_mon.speed = (u16)(((u32)(pbase->speed + dv_speed(s_battle.player_mon.dv)) * 2 * 6) / 100 + 5);
                s_battle.player_mon.special = (u16)(((u32)(pbase->special + dv_special(s_battle.player_mon.dv)) * 2 * 6) / 100 + 5);
                s_battle.player_mon.current_hp += s_battle.player_mon.max_hp - old_max;

                clear_lower_ui();
                char *p = str_append(s_msg_buf, mon_display_name(&s_battle.player_mon));
                p = str_append(p, " grew to\nlevel 6!");
                *p = '\0';
                battle_msg(s_msg_buf);
                s_battle.state = BS_LEVEL_UP;
            } else {
                s_battle.state = BS_END;
            }
        }
        break;

    case BS_LEVEL_UP:
        if (dialog_update()) {
            s_battle.state = BS_END;
        }
        break;

    case BS_DEFEAT:
        if (dialog_update()) {
            clear_lower_ui();
            battle_msg("[NAME] is out of\nusable POKeMON!\f[NAME] blacked out!");
            s_battle.state = BS_DEFEAT_WAIT;
        }
        break;

    case BS_DEFEAT_WAIT:
        if (dialog_update()) {
            s_battle.state = BS_END;
        }
        break;

    case BS_END:
        if (dialog_is_open()) { dialog_update(); break; }
        flags_set(FLAG_BATTLED_RIVAL_IN_OAKS_LAB);
        text_init();
        tilemap_rebuild();
        tilemap_update_scroll();
        tilemap_load_player_sprite();
        REG_DISPCNT = s_battle.saved_dispcnt;
        game_change_state(GAME_STATE_OVERWORLD);
        break;

    case BS_VICTORY_WAIT:
    case BS_EXP_WAIT:
    case BS_LEVEL_UP_WAIT:
        break;
    }
}
