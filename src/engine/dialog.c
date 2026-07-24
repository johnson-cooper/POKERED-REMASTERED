#include "dialog.h"
#include "game.h"
#include "text.h"
#include "input.h"
#include "audio.h"
#include "gba.h"

// Dialog box layout on BG0:
//   Box border: rows 14-17 (4 rows), cols 0-29 (30 cols)
//   Text area:  rows 15-16 (2 lines), cols 1-28 (28 chars per line)
// Border tiles use palette 0 color 3 (blue) for the border.

#define BOX_ROW     14
#define BOX_ROWS    4
#define BOX_COLS    30
#define TEXT_LINE1  (BOX_ROW + 1)
#define TEXT_LINE2  (BOX_ROW + 2)
#define TEXT_WIDTH  28  // cols 1..28
#define TEXT_START_COL 1

// Border tiles — we repurpose specific tile indices for border drawing.
// Using pokered's box border tile codes:
//   0xEC=top-left, 0xEE=top, 0xED=top-right
//   0xF0=left,     0x7F=fill, 0xF2=right (or just use blank + border color)
// We draw the box using filled characters in palette colors.
// Simple implementation: draw '+', '-', '|' mapped to special tiles.
// For now use spaces with a colored background — we'll use palette trick:
// We set tile index 0x7F (space) as the box fill, and draw explicit borders.
// The box will just be a white tile region surrounded by colored border tiles.

// Simpler: just use text characters for the border.
#define TILE_BORDER_H  0xF5  // pokered horizontal bar
#define TILE_BORDER_V  0xF4  // pokered vertical bar (or just use space with palette)

typedef enum {
    DIALOG_CLOSED = 0,
    DIALOG_PRINTING,
    DIALOG_WAITING_A,
    DIALOG_DONE,
} DialogState;

typedef enum {
    YESNO_NONE = 0,
    YESNO_OPEN,
} YesNoState;

static DialogState  s_state    = DIALOG_CLOSED;
static const char  *s_text_ptr = NULL;
static bool8        s_page_more = FALSE;
static u8           s_cursor   = 0; // which YES/NO option
static YesNoState   s_yesno    = YESNO_NONE;
static u8           s_blink    = 0;

static char s_dialog_text[512];

static void expand_player_name(const char *src) {
    static const char token[] = "[NAME]";
    static const char rival_token[] = "[RIVAL]";
    static const char plain_rival[] = "RIVAL";
    const char *name = game_get_player_name();
    const char *rival = game_get_rival_name();
    u32 out = 0;

    while (*src && out < sizeof(s_dialog_text) - 1) {
        bool8 is_token = TRUE;
        for (u32 i = 0; token[i]; i++) {
            if (src[i] != token[i]) {
                is_token = FALSE;
                break;
            }
        }

        if (is_token && name[0]) {
            for (u32 i = 0; name[i] && out < sizeof(s_dialog_text) - 1; i++)
                s_dialog_text[out++] = name[i];
            src += sizeof(token) - 1;
            continue;
        }

        is_token = TRUE;
        for (u32 i = 0; rival_token[i]; i++) {
            if (src[i] != rival_token[i]) {
                is_token = FALSE;
                break;
            }
        }

        if (is_token && rival[0]) {
            for (u32 i = 0; rival[i] && out < sizeof(s_dialog_text) - 1; i++)
                s_dialog_text[out++] = rival[i];
            src += sizeof(rival_token) - 1;
        } else {
            is_token = TRUE;
            for (u32 i = 0; plain_rival[i]; i++) {
                if (src[i] != plain_rival[i]) {
                    is_token = FALSE;
                    break;
                }
            }

            // Replace the standalone uppercase placeholder, but do not
            // alter words such as RIVALRY.
            char next = src[sizeof(plain_rival) - 1];
            bool8 word_end = (next == '\0' ||
                              next < 'A' || next > 'Z');
            if (is_token && word_end && rival[0]) {
                for (u32 i = 0; rival[i] && out < sizeof(s_dialog_text) - 1; i++)
                    s_dialog_text[out++] = rival[i];
                src += sizeof(plain_rival) - 1;
            } else {
                s_dialog_text[out++] = *src++;
            }
        }
    }
    s_dialog_text[out] = '\0';
}

// Draw the box outline using dedicated box-drawing tiles (BOX_TL..BOX_FILL).
static void draw_box(u8 row, u8 col, u8 rows, u8 cols) {
    // Top row
    text_draw_tile(col, row, BOX_TL);
    for (u8 c = col+1; c < col+cols-1; c++) text_draw_tile(c, row, BOX_TE);
    text_draw_tile(col+cols-1, row, BOX_TR);
    // Middle rows: opaque white interior
    for (u8 r = row+1; r < row+rows-1; r++) {
        text_draw_tile(col, r, BOX_LE);
        for (u8 c = col+1; c < col+cols-1; c++) text_draw_tile(c, r, BOX_FILL);
        text_draw_tile(col+cols-1, r, BOX_RE);
    }
    // Bottom row
    text_draw_tile(col, row+rows-1, BOX_BL);
    for (u8 c = col+1; c < col+cols-1; c++) text_draw_tile(c, row+rows-1, BOX_BE);
    text_draw_tile(col+cols-1, row+rows-1, BOX_BR);
}

static void clear_box(u8 row, u8 col, u8 rows, u8 cols) {
    // Restore all box positions to transparent so the map shows through.
    for (u8 r = row; r < row+rows; r++)
        for (u8 c = col; c < col+cols; c++)
            text_draw_tile(c, r, TEXT_BLANK_TILE);
}

// Print up to two lines of text from s_text_ptr.
// Returns TRUE if there is more text after this page.
static bool8 print_page(void) {
    if (!s_text_ptr || !*s_text_ptr) return FALSE;

    // Clear text area with opaque fill so map doesn't show through
    for (u8 c = TEXT_START_COL; c < TEXT_START_COL + TEXT_WIDTH; c++) {
        text_draw_tile(c, TEXT_LINE1, BOX_FILL);
        text_draw_tile(c, TEXT_LINE2, BOX_FILL);
    }

    u8 col = TEXT_START_COL;
    u8 row = TEXT_LINE1;

    while (*s_text_ptr && row <= TEXT_LINE2) {
        char ch = *s_text_ptr;

        // Newline: move to next line
        if (ch == '\n') {
            s_text_ptr++;
            row++;
            col = TEXT_START_COL;
            continue;
        }

        // Page break (two newlines or ^P marker): stop here, leave on page
        if (ch == '\f') {
            s_text_ptr++; // skip
            return TRUE;  // more text follows
        }

        // Word-wrap: if we'd overflow the line
        if (col >= TEXT_START_COL + TEXT_WIDTH) {
            row++;
            col = TEXT_START_COL;
            if (row > TEXT_LINE2) break;
        }

        text_draw_char(col, row, ch);
        col++;
        s_text_ptr++;
    }

    return (*s_text_ptr != '\0');
}

void dialog_open(void) {
    draw_box(BOX_ROW, 0, BOX_ROWS, BOX_COLS);
    s_state = DIALOG_PRINTING;
    s_text_ptr = NULL;
    s_page_more = FALSE;
    s_blink = 0;
}

void dialog_set_text(const char *s) {
    expand_player_name(s);
    s_text_ptr = s_dialog_text;
    s_page_more = print_page();
    s_state = DIALOG_WAITING_A;
    s_blink = 0;
}

bool8 dialog_update(void) {
    if (s_state == DIALOG_CLOSED || s_state == DIALOG_DONE)
        return TRUE;

    if (s_state == DIALOG_WAITING_A) {
        // Blinking arrow
        s_blink = (s_blink + 1) & 63;
        if (s_blink < 32)
            text_draw_char(TEXT_START_COL + TEXT_WIDTH - 1, TEXT_LINE2, 'v');
        else
            text_draw_tile(TEXT_START_COL + TEXT_WIDTH - 1, TEXT_LINE2, BOX_FILL);

        if (input_pressed(KEY_A) || input_pressed(KEY_B)) {
            audio_sfx_play(input_pressed(KEY_A) ? AUDIO_SFX_TEXT : AUDIO_SFX_CANCEL);
            // A final page must remain visible after it is drawn. Only close
            // when the player presses A again while that final page is shown.
            if (!s_page_more) {
                // No more text: close box
                clear_box(BOX_ROW, 0, BOX_ROWS, BOX_COLS);
                s_state = DIALOG_DONE;
                s_text_ptr = NULL;
                s_page_more = FALSE;
                return TRUE;
            }
            s_page_more = print_page();
            // More pages, or the newly displayed final page; stay visible.
            s_blink = 0;
        }
    }

    return FALSE;
}

bool8 dialog_is_open(void) {
    return s_state != DIALOG_CLOSED && s_state != DIALOG_DONE;
}

void dialog_close(void) {
    clear_box(BOX_ROW, 0, BOX_ROWS, BOX_COLS);
    dialog_yesno_close();
    s_state = DIALOG_CLOSED;
    s_text_ptr = NULL;
    s_page_more = FALSE;
}

// ── YES / NO menu ─────────────────────────────────────────────────────────────
// Drawn as a small box at col 22, row 9-13 (above dialog box)
#define YESNO_COL  22
#define YESNO_ROW  9
#define YESNO_W    7
#define YESNO_H    4

void dialog_yesno_open(void) {
    s_yesno  = YESNO_OPEN;
    s_cursor = 0;
    draw_box(YESNO_ROW, YESNO_COL, YESNO_H, YESNO_W);
    text_draw_str(YESNO_COL + 2, YESNO_ROW + 1, "YES");
    text_draw_str(YESNO_COL + 2, YESNO_ROW + 2, "NO");
    text_draw_char(YESNO_COL + 1, YESNO_ROW + 1 + s_cursor, '>');
}

void dialog_yesno_close(void) {
    if (s_yesno == YESNO_OPEN)
        clear_box(YESNO_ROW, YESNO_COL, YESNO_H, YESNO_W);
    s_yesno = YESNO_NONE;
}

u8 dialog_yesno_update(void) {
    if (s_yesno != YESNO_OPEN) return 0xFF;

    u8 old = s_cursor;
    if (input_pressed(KEY_DOWN) && s_cursor < 1) s_cursor++;
    if (input_pressed(KEY_UP)   && s_cursor > 0) s_cursor--;

    if (s_cursor != old) {
        audio_sfx_play(AUDIO_SFX_SELECT);
        text_draw_char(YESNO_COL + 1, YESNO_ROW + 1 + old,    ' ');
        text_draw_char(YESNO_COL + 1, YESNO_ROW + 1 + s_cursor, '>');
    }

    if (input_pressed(KEY_A)) {
        audio_sfx_play(AUDIO_SFX_CONFIRM);
        u8 result = (s_cursor == 0) ? 1 : 0;
        clear_box(YESNO_ROW, YESNO_COL, YESNO_H, YESNO_W);
        s_yesno = YESNO_NONE;
        return result;
    }
    if (input_pressed(KEY_B)) {
        audio_sfx_play(AUDIO_SFX_CANCEL);
        clear_box(YESNO_ROW, YESNO_COL, YESNO_H, YESNO_W);
        s_yesno = YESNO_NONE;
        return 0; // B = NO
    }

    return 0xFF; // pending
}
