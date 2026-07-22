#pragma once
#include "types.h"

// BG0 (SBB 20, CBB 2) tile-based text layer.
// 30 columns × 20 rows of 8×8 tiles.

#define TEXT_COLS 30
#define TEXT_ROWS 20
#define TEXT_PAL   15  // UI palette bank used by the existing text system

// Box-drawing tiles loaded into CBB2 by text_init (indices 128-136).
// Use text_draw_tile() to write these directly, bypassing ASCII mapping.
#define BOX_TL   128  // top-left corner
#define BOX_TE   129  // top edge
#define BOX_TR   130  // top-right corner
#define BOX_LE   131  // left edge
#define BOX_RE   132  // right edge
#define BOX_BL   133  // bottom-left corner
#define BOX_BE   134  // bottom edge
#define BOX_BR   135  // bottom-right corner
#define BOX_FILL 136  // opaque white interior fill
#define TEXT_BLANK_TILE 137 // transparent tile used only when clearing UI
#define HP_BAR_FILL_TILE 138
#define HP_BAR_EMPTY_TILE 139

void text_init(void);
void text_clear(void);
void text_fill_opaque(void);
void text_draw_tile(u8 col, u8 row, u8 tile_idx);
void text_draw_tile_pal(u8 col, u8 row, u8 tile_idx, u8 palette);
void text_draw_char(u8 col, u8 row, char c);
void text_draw_str(u8 col, u8 row, const char *s);
void text_draw_str_pal(u8 col, u8 row, const char *s, u8 palette);
void text_draw_str_n(u8 col, u8 row, const char *s, u8 n);
