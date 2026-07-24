#include "text.h"
#include "gba.h"
#include "gfx_font.h"

// Font tiles are loaded into CBB2 (0x06008000), starting at tile 0.
// The pokered font has 128 characters in a 16×8 arrangement (128×64 image).
// The converted pokered font is a compact 16×8 glyph sheet, not a 256-tile
// character-code table. Uppercase A-P occupy tiles 0-15 and Q-Z occupy 16-25.
// BG0 uses palette 0 from the UI palette area (0x05000000).

#define FONT_CBB        1
#define BG0_SBB        20
#define UI_PAL_BASE    ((vu16*)0x05000000)
#define UI_VRAM_BASE   ((vu16*)(MEM_VRAM + 0x10000))  // CBB2 = 0x06010000? No...

// CBB2 starts at VRAM offset 0x10000 (each CBB = 0x4000 tiles × 32 bytes = 0x8000 bytes,
// but GBA CBBs are 16KB = 0x4000 each). Wait: CBB index × 0x4000 = byte offset from VRAM base.
// CBB2 = 0x06000000 + 2*0x4000 = 0x06008000.
#define FONT_VRAM_ADDR  ((u32*)(MEM_VRAM + FONT_CBB * 0x4000))

// SBB20 screenblock: offset = SBB * 0x800 bytes from VRAM base.
// SBB20 = 0x06000000 + 20*0x800 = 0x06000000 + 0xA000 = 0x0600A000.
#define BG0_SBB_ADDR   ((vu16*)(MEM_VRAM + BG0_SBB * 0x800))

// Font palette: white background (index 0=transparent on BG but we want
// opaque UI), index 1=black text, index 15=white bg.
// We set BG0 palette slot 0 (first 16 colors at 0x05000000).
static const u16 s_font_pal[16] = {
    0x7FFF, // 0 = transparent (shows backdrop / map behind)
    0x0000, // 1 = black (text strokes, border lines)
    0x7FFF, // 2 = opaque white (text tile background — keeps text readable on any BG)
    0x56B5, // 3 = mid blue (unused)
    0x7FFF, // 4 = opaque white (dialog box fill tile)
    0x7FFF, 0x7FFF, 0x7FFF,
    0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
    0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF,
};

// Tile index used for blank/space — row 4 col 0 of the font image (tile 64).
// We explicitly zero it out so it is always transparent.
#define BLANK_TILE TEXT_BLANK_TILE

void text_init(void) {
    // The title screen exists before a world/map is loaded, so configure and
    // enable BG0 here rather than waiting for tilemap_init().
    REG_BG0CNT = (u16)(BG_CBB(FONT_CBB) | BG_SBB(20) |
                       BG_4BPP | BG_SIZE_256x256 | 0);
    REG_DISPCNT |= DCNT_BG0;

    // Load font tiles into CBB2.
    // grit output: gfx_fontTiles[1024] words = 128 tiles × 8 words/tile × 4bpp.
    u32 *dst = FONT_VRAM_ADDR;
    const u32 *src = (const u32 *)gfx_fontTiles;
    for (u32 i = 0; i < gfx_fontTilesLen / 4; i++)
        dst[i] = src[i];

    // Fix font tile encoding. grit used the image's black color as its transparent
    // marker, so grit assigned character strokes (black) → nibble 0 and character
    // backgrounds (white) → nibble 1. We remap:
    //   0 → 1  (stroke: move to palette index 1 = 0x0000 = black)
    //   1 → 2  (background: move to palette index 2 = 0x7FFF = opaque white)
    // This makes fonts render as black text on an opaque-white background.
    for (u32 i = 0; i < 128 * 8; i++) {
        u32 word = dst[i];
        u32 result = 0;
        for (u32 shift = 0; shift < 32; shift += 4) {
            u32 n = (word >> shift) & 0xF;
            if (n == 0) n = 1;
            else if (n == 1) n = 2;
            result |= (n << shift);
        }
        dst[i] = result;
    }
    // BLANK_TILE must be all-transparent (index 0) so text_clear() doesn't
    // cover the map with opaque tiles.
    for (u32 w = 0; w < 8; w++)
        dst[BLANK_TILE * 8 + w] = 0;

    // Box-drawing and battle HP tiles at indices 128-146.
    // In 4bpp: each row = 4 bytes; low nibble = left pixel, high = right pixel.
    // Palette index 1 (0x1) = black border line; 2 (0x2) = opaque white fill.
    // Pixel row pattern helpers:
    //   full black row:  0x11111111
    //   left col only:   0x00000001  ([1,0,0,0,0,0,0,0])
    //   right col only:  0x10000000  ([0,0,0,0,0,0,0,1])
    //   all fill (white):0x22222222
    static const u32 s_box_tiles[19][8] = {
        /* 128 BOX_TL */ {0x11111111,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221},
        /* 129 BOX_TE */ {0x11111111,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222},
        /* 130 BOX_TR */ {0x11111111,0x12222222,0x12222222,0x12222222,0x12222222,0x12222222,0x12222222,0x12222222},
        /* 131 BOX_LE */ {0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221},
        /* 132 BOX_RE */ {0x12222222,0x12222222,0x12222222,0x12222222,0x12222222,0x12222222,0x12222222,0x12222222},
        /* 133 BOX_BL */ {0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x11111111},
        /* 134 BOX_BE */ {0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x11111111},
        /* 135 BOX_BR */ {0x12222222,0x12222222,0x12222222,0x12222222,0x12222222,0x12222222,0x12222222,0x11111111},
        /* 136 BOX_FILL*/       {0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222},
        /* 137 TEXT_BLANK_TILE */ {0,0,0,0,0,0,0,0},
        /* 138 HP_BAR_FILL_TILE */ {0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111,0x11111111},
        /* 139 HP_BAR_EMPTY_TILE */ {0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222,0x22222222},
        /* 140-146: 1-7 filled pixels, then lavender background pixels */
        /* 140 */ {0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221,0x22222221},
        /* 141 */ {0x22222211,0x22222211,0x22222211,0x22222211,0x22222211,0x22222211,0x22222211,0x22222211},
        /* 142 */ {0x22222111,0x22222111,0x22222111,0x22222111,0x22222111,0x22222111,0x22222111,0x22222111},
        /* 143 */ {0x22221111,0x22221111,0x22221111,0x22221111,0x22221111,0x22221111,0x22221111,0x22221111},
        /* 144 */ {0x22211111,0x22211111,0x22211111,0x22211111,0x22211111,0x22211111,0x22211111,0x22211111},
        /* 145 */ {0x22111111,0x22111111,0x22111111,0x22111111,0x22111111,0x22111111,0x22111111,0x22111111},
        /* 146 */ {0x21111111,0x21111111,0x21111111,0x21111111,0x21111111,0x21111111,0x21111111,0x21111111},
    };
    u32 *box_dst = FONT_VRAM_ADDR + 128 * 8;
    for (u32 t = 0; t < 19; t++)
        for (u32 w = 0; w < 8; w++)
            box_dst[t * 8 + w] = s_box_tiles[t][w];

    // Full-black tile used by the battle-entry spiral. It lives after the
    // normal UI/HP tiles and uses palette index 1 (black).
    for (u8 w = 0; w < 8; w++)
        box_dst[19 * 8 + w] = 0x11111111;


    // Load UI palette into bank TEXT_PAL.
    vu16 *pal = UI_PAL_BASE + TEXT_PAL * 16;
    for (u32 i = 0; i < 16; i++)
        pal[i] = s_font_pal[i];

    // Dedicated Pokédex palette. It is separate from both the map palette
    // bank and the normal dialogue palette.
    vu16 *pokedex_pal = UI_PAL_BASE + 14 * 16;
    for (u32 i = 0; i < 16; i++) pokedex_pal[i] = s_font_pal[i];
    pokedex_pal[1] = RGB15(2, 1, 2);
    pokedex_pal[2] = RGB15(31, 25, 31);
    pokedex_pal[3] = RGB15(8, 18, 8);
    pokedex_pal[4] = RGB15(20, 25, 20);

    // Species-specific front-sprite palettes. Palette 14 remains reserved
    // for the Pokédex paper and text.
    vu16 *bulba_pal = UI_PAL_BASE + 11 * 16;
    vu16 *char_pal  = UI_PAL_BASE + 12 * 16;
    vu16 *squirt_pal = UI_PAL_BASE + 13 * 16;
    for (u8 i = 0; i < 16; i++) {
        bulba_pal[i] = pokedex_pal[i];
        char_pal[i] = pokedex_pal[i];
        squirt_pal[i] = pokedex_pal[i];
    }
    bulba_pal[3] = RGB15(5, 15, 4);
    bulba_pal[4] = RGB15(18, 28, 12);
    char_pal[3] = RGB15(20, 5, 2);
    char_pal[4] = RGB15(31, 16, 4);
    squirt_pal[3] = RGB15(3, 9, 22);
    squirt_pal[4] = RGB15(13, 23, 31);

    // BG backdrop shows through transparent pixels — set to white.
    UI_PAL_BASE[0] = 0x7FFF;

    text_clear();
}

void text_clear(void) {
    vu16 *sbb = BG0_SBB_ADDR;
    for (u32 i = 0; i < 32 * 32; i++)
        sbb[i] = (u16)(BLANK_TILE | (TEXT_PAL << 12));
}

void text_fill_opaque(void) {
    vu16 *sbb = BG0_SBB_ADDR;
    for (u32 i = 0; i < 32 * 32; i++)
        sbb[i] = (u16)(BOX_FILL | (TEXT_PAL << 12));
}

// Map ASCII character to grit tile index.
// pokered font.png is 128×64 (16 tiles wide × 8 rows = 128 tiles).
// The font encoding matches the pokered text encoding where 0x80=A, etc.
// For simplicity, we map printable ASCII to the nearest available glyph.
// pokered tile layout (each tile row = 16 chars):
//   Tiles 0-15:  pokered chars 0x00-0x0F
//   Tiles 16-31: pokered chars 0x10-0x1F
//   ...
// We want ASCII ' '=0x20, 'A'=0x41, 'a'=0x61, '0'=0x30, etc.
// pokered font: uppercase A-Z at tile rows 0x80-0x99 (pokered encoding),
// but in the PNG they're at specific positions.
// Practical approach: treat grit tile index = pokered char code.
// pokered char 0x80='A', 0x81='B', ..., 0x99='Z',
//              0x9A='(', 0x9B=')', 0x9C=':', 0x9D=';', 0x9E='[', 0x9F=']',
//              0xA0='a', ..., 0xB9='z',
//              0xBA='é', 0xE0='\'', 0xE3='-', 0xE6='?',
//              0xE7='!', 0xE8='.', 0xF4=',', 0xF6='0', ..., 0xFF='9'.
//              0x7F=' ' (blank tile)
// Font tile layout (16 tiles wide × 8 rows, confirmed from screenshot):
//   Row 0 (0-15):   A B C D E F G H I J K L M N O P
//   Row 1 (16-31):  Q R S T U V W X Y Z ( ) : ; [ ]
//   Row 2 (32-47):  a b c d e f g h i j k l m n o p
//   Row 3 (48-63):  q r s t u v w x y z é 'd 'l 's 't 'v
//   Rows 4-6:       other glyphs / punctuation
//   Row 7 (112-127):... (118) 0 1 2 3 4 5 6 7 8 9
// BLANK_TILE (64) is zeroed out by text_init.
static u8 ascii_to_pokered(char c) {
    if (c >= 'A' && c <= 'P') return (u8)(c - 'A');         // 0-15
    if (c >= 'Q' && c <= 'Z') return (u8)(16 + c - 'Q');    // 16-25
    if (c >= 'a' && c <= 'p') return (u8)(32 + c - 'a');    // 32-47
    if (c >= 'q' && c <= 'z') return (u8)(48 + c - 'q');    // 48-57
    if (c >= '0' && c <= '9') return (u8)(118 + c - '0');   // 118-127
    switch (c) {
        case '(':  return 26;
        case ')':  return 27;
        case ':':  return 28;
        case ';':  return 29;
        case '[':  return 30;
        case ']':  return 31;
        case '~':  return 58;  // pokered $BA = é
        case '\'': return 96;  // pokered $E0
        case '-':  return 99;  // pokered $E3
        case '?':  return 102; // pokered $E6
        case '!':  return 103; // pokered $E7
        case '.':  return 104; // pokered $E8
        case ',':  return 116; // pokered $F4
        case '>':  return 108; // pokered right-arrow glyph
        default:   return BLANK_TILE;
    }
}

void text_draw_tile(u8 col, u8 row, u8 tile_idx) {
    if (col >= TEXT_COLS || row >= TEXT_ROWS) return;
    BG0_SBB_ADDR[row * 32 + col] = (u16)(tile_idx | (TEXT_PAL << 12));
}

void text_draw_tile_pal(u8 col, u8 row, u8 tile_idx, u8 palette) {
    if (col >= TEXT_COLS || row >= TEXT_ROWS) return;
    BG0_SBB_ADDR[row * 32 + col] = (u16)(tile_idx | (palette << 12));
}

void text_draw_char(u8 col, u8 row, char c) {
    if (col >= TEXT_COLS || row >= TEXT_ROWS) return;
    // Spaces inside a dialog must preserve the opaque white window. Clearing
    // the UI uses TEXT_BLANK_TILE directly instead of going through here.
    if (c == ' ') {
        text_draw_tile(col, row, BOX_FILL);
        return;
    }
    u8 tile = ascii_to_pokered(c);
    // Tile entry: tile_index | (palette << 12)
    BG0_SBB_ADDR[row * 32 + col] = (u16)(tile | (TEXT_PAL << 12));
}

void text_draw_str(u8 col, u8 row, const char *s) {
    while (*s && col < TEXT_COLS) {
        text_draw_char(col++, row, *s++);
    }
}

void text_draw_str_pal(u8 col, u8 row, const char *s, u8 palette) {
    while (*s && col < TEXT_COLS) {
        if (row < TEXT_ROWS) {
            u8 tile = (*s == ' ') ? BOX_FILL : ascii_to_pokered(*s);
            BG0_SBB_ADDR[row * 32 + col] = (u16)(tile | (palette << 12));
        }
        col++;
        s++;
    }
}

void text_draw_str_n(u8 col, u8 row, const char *s, u8 n) {
    for (u8 i = 0; i < n && *s && col < TEXT_COLS; i++) {
        text_draw_char(col++, row, *s++);
    }
}
