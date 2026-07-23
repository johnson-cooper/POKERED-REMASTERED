#include "world.h"
#include "gba.h"
#include "gfx_overworld.h"
#include "gfx_player.h"
#include "gfx_pokeball.h"
#include "gfx_npcs.h"
#include "gfx_npcs_extra.h"

// VRAM layout:
// CBB 0 (SBBs 0-7,  0x06000000): map tile graphics
// CBB 2 (SBBs 16-23,0x06008000): UI tile graphics
// BG2 SBBs 24-27: 512x512 bottom map layer
// BG1 SBBs 28-31: 512x512 top map layer
// BG0 SBB  20:    UI

#define MAP_TILE_CBB    0
#define BG2_SBB         24
#define BG1_SBB         28
#define UI_SBB          20

#define DISPCNT_WIN0 0x2000

#define SBB_PTR(n)  ((vu16*)(MEM_VRAM + (n) * 0x800))

#define TILE_ENTRY(idx, pal, hflip, vflip) \
    ((u16)((idx) | ((hflip) ? 0x0400 : 0) | ((vflip) ? 0x0800 : 0) | ((pal) << 12)))

// Write one entry into a 512x512 BG made from a 2x2 grid of screenblocks.
static inline void sbb_set(u32 sbb, u32 sx, u32 sy, u16 entry) {
    u32 block_x = (sx >> 5) & 1;
    u32 block_y = (sy >> 5) & 1;
    u32 block = sbb + block_x + block_y * 2;
    SBB_PTR(block)[(sy & 31) * 32 + (sx & 31)] = entry;
}

static void write_metatile(s32 mx, s32 my) {
    const MapLayout *layout = g_world.map->layout;
    if (!layout || !layout->tileset)
        return;

    MapCell cell = map_get_cell(mx, my);
    u16 mtid = MAPCELL_METATILE(cell);

    if (mtid >= layout->tileset->metatile_count)
        return;

    const Metatile *mt = &layout->tileset->metatiles[mtid];

    // Each Pokemon Red block is 4x4 8px tiles, or 32x32 pixels.
    u32 tx = (u32)(mx * 4);
    u32 ty = (u32)(my * 4);

    for (u32 row = 0; row < 4; row++) {
        for (u32 col = 0; col < 4; col++) {
            u32 i = row * 4 + col;
            u16 tile_id = mt->bottom[i];
            u8 pal = mt->palettes[i];
            sbb_set(BG2_SBB, tx + col, ty + row,
                    TILE_ENTRY(tile_id, pal, 0, 0));

            tile_id = mt->top[i];
            pal = mt->top_palettes[i];
            sbb_set(BG1_SBB, tx + col, ty + row,
                    tile_id ? TILE_ENTRY(tile_id, pal, 0, 0) : 0);
        }
    }
}

static inline u32 remap_overlay_nibbles(u32 v) {
    u32 result = 0;
    for (u32 shift = 0; shift < 32; shift += 4) {
        u32 n = (v >> shift) & 0xF;

        // White is the "background" shade in the original monochrome tiles.
        // Make it transparent on BG1. Preserve black outlines by moving them
        // from palette index 0 to index 1, because index 0 is transparent.
        if (n == 0xF)
            n = 0;
        else if (n == 0)
            n = 1;

        result |= n << shift;
    }
    return result;
}

static inline u8 tile_pixel(const u32 rows[8], u32 x, u32 y) {
    return (u8)((rows[y] >> (x * 4)) & 0xF);
}

static inline void set_tile_pixel(u32 rows[8], u32 x, u32 y, u8 value) {
    u32 shift = x * 4;
    rows[y] = (rows[y] & ~(0xFu << shift)) | ((u32)value << shift);
}

static void make_edge_mask_overlay(u32 dst[8], const u32 src[8]) {
    bool8 transparent[8][8] = { FALSE };
    u8 queue_x[64];
    u8 queue_y[64];
    u32 head = 0;
    u32 tail = 0;

    for (u32 y = 0; y < 8; y++) {
        for (u32 x = 0; x < 8; x++) {
            bool8 edge = (x == 0 || y == 0 || x == 7 || y == 7);
            if (edge && tile_pixel(src, x, y) == 0xF) {
                transparent[y][x] = TRUE;
                queue_x[tail] = (u8)x;
                queue_y[tail] = (u8)y;
                tail++;
            }
        }
    }

    while (head < tail) {
        s32 x = queue_x[head];
        s32 y = queue_y[head];
        head++;

        static const s8 dx[4] = { -1, 1, 0, 0 };
        static const s8 dy[4] = { 0, 0, -1, 1 };
        for (u32 i = 0; i < 4; i++) {
            s32 nx = x + dx[i];
            s32 ny = y + dy[i];
            if (nx < 0 || ny < 0 || nx >= 8 || ny >= 8)
                continue;
            if (!transparent[ny][nx] && tile_pixel(src, (u32)nx, (u32)ny) == 0xF) {
                transparent[ny][nx] = TRUE;
                queue_x[tail] = (u8)nx;
                queue_y[tail] = (u8)ny;
                tail++;
            }
        }
    }

    for (u32 y = 0; y < 8; y++) {
        dst[y] = 0;
        for (u32 x = 0; x < 8; x++) {
            u8 n = transparent[y][x] ? 0 : tile_pixel(src, x, y);
            if (!transparent[y][x] && n == 0)
                n = 1;
            set_tile_pixel(dst, x, y, n);
        }
    }
}

// The four sloped roof-corner tiles contain both outside white pixels and a
// light roof strip. The flood mask correctly removes the outside area, but
// that strip can be connected to it in the source art. Restore the lower
// roof-facing rows so grass cannot show through as a green seam.
static void make_roof_corner_overlay(u32 dst[8], const u32 src[8]) {
    make_edge_mask_overlay(dst, src);

    for (u32 y = 0; y < 8; y++) {
        for (u32 x = 0; x < 8; x++) {
            if (tile_pixel(src, x, y) == 0xF)
                set_tile_pixel(dst, x, y, 0xF);
        }
    }
}

// A building tile is an opaque object tile. The original GB artwork uses
// white for both empty space and light wall pixels, but building blocks are
// already clipped to their exact 8x8 cells by the block layout. Keeping every
// source pixel opaque prevents the grass palette from bleeding through roofs,
// walls, windows, and doors.
static void make_solid_overlay(u32 dst[8], const u32 src[8]) {
    for (u32 y = 0; y < 8; y++) {
        u32 row = src[y];
        u32 result = 0;
        for (u32 x = 0; x < 8; x++) {
            u32 n = (row >> (x * 4)) & 0xF;
            if (n == 0) n = 1;
            result |= n << (x * 4);
        }
        dst[y] = result;
    }
}

void tilemap_load_tileset(const Tileset *ts) {
    if (!ts)
        return;

    // 8 u32 words per 4bpp 8x8 tile.
    u32 words = ts->tile_count * 8;
    dma_copy32((void*)MEM_VRAM, ts->tiles, words);

    // Tile 0 is the transparent tile for the unused top BG layer.
    dma_fill32((void*)MEM_VRAM, 0, 8);

    // Build transparent BG1 overlay copies after the original tile graphics.
    u32 *vram_tiles = (u32*)MEM_VRAM;
    u32 overlay_word = OVERWORLD_OVERLAY_TRANSPARENT_WHITE_BASE * 8;
    for (u32 i = 0; i < words; i++)
        vram_tiles[overlay_word + i] = remap_overlay_nibbles(ts->tiles[i]);

    overlay_word = OVERWORLD_OVERLAY_EDGE_MASK_BASE * 8;
    for (u32 tile = 0; tile < ts->tile_count; tile++) {
        if (tile == 0x05 || tile == 0x06 ||
            tile == 0x08 || tile == 0x09)
            make_roof_corner_overlay(&vram_tiles[overlay_word + tile * 8],
                                     &ts->tiles[tile * 8]);
        else
            make_edge_mask_overlay(&vram_tiles[overlay_word + tile * 8],
                                   &ts->tiles[tile * 8]);
    }

    overlay_word = OVERWORLD_OVERLAY_SOLID_BASE * 8;
    for (u32 tile = 0; tile < ts->tile_count; tile++)
        make_solid_overlay(&vram_tiles[overlay_word + tile * 8],
                           &ts->tiles[tile * 8]);

    for (u32 i = 0; i < ts->palette_count; i++) {
        const u16 *src = &ts->palettes[i * 16];
        u16 *dst = (u16*)MEM_PAL + i * 16;
        for (u32 j = 0; j < 16; j++)
            dst[j] = src[j];
    }
}

// Swap nibbles 0 and F in a u32.
// grit puts white background at nibble F; GBA OBJ transparency needs nibble 0.
static inline u32 remap_sprite_nibbles(u32 v) {
    u32 result = v;
    for (u32 shift = 0; shift < 32; shift += 4) {
        u32 n = (v >> shift) & 0xF;
        if (n == 0x0 || n == 0xF)
            result ^= (0xFu << shift);
    }
    return result;
}

void tilemap_load_player_sprite(void) {
    u32 *obj_vram = (u32*)(MEM_VRAM + 0x10000);
    for (u32 i = 0; i < g_player_tile_count * 8; i++)
        obj_vram[i] = remap_sprite_nibbles(g_player_tiles[i]);

    u32 pokeball_base = g_player_tile_count * 8;
    for (u32 i = 0; i < g_pokeball_tile_count * 8; i++)
        obj_vram[pokeball_base + i] = remap_sprite_nibbles(g_pokeball_tiles[i]);

    u32 blue_base = (g_player_tile_count + g_pokeball_tile_count) * 8;
    for (u32 i = 0; i < g_blue_tile_count * 8; i++)
        obj_vram[blue_base + i] = remap_sprite_nibbles(g_blue_tiles[i]);

    u32 oak_base = blue_base + g_blue_tile_count * 8;
    for (u32 i = 0; i < g_oak_tile_count * 8; i++)
        obj_vram[oak_base + i] = remap_sprite_nibbles(g_oak_tiles[i]);

    u32 girl_base = oak_base + g_oak_tile_count * 8;
    for (u32 i = 0; i < 24 * 8; i++)
        obj_vram[girl_base + i] = remap_sprite_nibbles(g_girl_tiles[i]);

    u32 fisher_base = girl_base + 24 * 8;
    for (u32 i = 0; i < 24 * 8; i++)
        obj_vram[fisher_base + i] = remap_sprite_nibbles(g_fisher_tiles[i]);

    u32 daisy_base = fisher_base + 24 * 8;
    for (u32 i = 0; i < 24 * 8; i++)
        obj_vram[daisy_base + i] = remap_sprite_nibbles(g_daisy_tiles[i]);

    u32 pokedex_base = daisy_base + 24 * 8;
    for (u32 i = 0; i < 4 * 8; i++)
        obj_vram[pokedex_base + i] = remap_sprite_nibbles(g_pokedex_overworld_tiles[i]);

    u32 mom_base = pokedex_base + 4 * 8;
    for (u32 i = 0; i < 24 * 8; i++)
        obj_vram[mom_base + i] = remap_sprite_nibbles(g_mom_tiles[i]);




    u16 *obj_pal = (u16*)0x05000200;
    obj_pal[ 0] = 0x0000;               // transparent
    obj_pal[ 5] = RGB15( 4, 4,16);      // dark navy
    obj_pal[10] = RGB15(28,18,10);      // skin tone
    obj_pal[15] = RGB15( 2, 2, 2);      // near-black outline
}

void tilemap_init(void) {
    REG_BG2CNT = (u16)(BG_CBB(MAP_TILE_CBB) | BG_SBB(BG2_SBB) |
                       BG_4BPP | BG_SIZE_512x512 | 2);

    REG_BG1CNT = (u16)(BG_CBB(MAP_TILE_CBB) | BG_SBB(BG1_SBB) |
                       BG_4BPP | BG_SIZE_512x512 | 1);

    REG_BG0CNT = (u16)(BG_CBB(1) | BG_SBB(UI_SBB) |
                       BG_4BPP | BG_SIZE_256x256 | 0);

    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_MAP_1D | DCNT_BG0 |
                  DCNT_BG1 | DCNT_BG2 | DCNT_OBJ;
}

void tilemap_rebuild(void) {
    const MapLayout *layout = g_world.map->layout;
    if (!layout)
        return;

    for (u32 i = 0; i < 4; i++) {
        dma_fill32((void*)SBB_PTR(BG2_SBB + i), 0, 32 * 32 / 2);
        dma_fill32((void*)SBB_PTR(BG1_SBB + i), 0, 32 * 32 / 2);
    }

    tilemap_load_tileset(layout->tileset);

    for (s32 y = 0; y < layout->height; y++) {
        for (s32 x = 0; x < layout->width; x++)
            write_metatile(x, y);
    }
}

static s32 s_last_cam_tx = -9999;
static s32 s_last_cam_ty = -9999;

void tilemap_update_scroll(void) {
    Camera *cam = &g_world.camera;
    const MapLayout *layout = g_world.map->layout;

    REG_BG2HOFS = (u16)(cam->x & 0x1FF);
    REG_BG2VOFS = (u16)(cam->y & 0x1FF);
    REG_BG1HOFS = (u16)(cam->x & 0x1FF);
    REG_BG1VOFS = (u16)(cam->y & 0x1FF);

    // Hardware windows prevent the 512x512 BG tilemap from wrapping into
    // the visible area when a small interior is centered with a negative
    // camera offset. Keep BG1/BG2/OBJ visible only inside the room rectangle;
    // leave BG0 available outside for the UI.
    if (layout && layout->width * 32 < 240 && layout->height * 32 < 160) {
        s32 left = -cam->x;
        s32 top = -cam->y;
        s32 right = left + layout->width * 32;
        s32 bottom = top + layout->height * 32;
        if (left < 0) left = 0;
        if (top < 0) top = 0;
        if (right > 240) right = 240;
        if (bottom > 160) bottom = 160;
        REG_WIN0H = (u16)(((left & 0xFF) << 8) | (right & 0xFF));
        REG_WIN0V = (u16)(((top & 0xFF) << 8) | (bottom & 0xFF));
        REG_WININ = 0x001F;  // BG0, BG1, BG2, BG3, and OBJ inside
        REG_WINOUT = 0x0001; // BG0 only outside
        REG_DISPCNT |= DISPCNT_WIN0;
    } else {
        REG_DISPCNT &= (u16)~DISPCNT_WIN0;
    }

    s32 cam_mx = cam->x / 32;
    s32 cam_my = cam->y / 32;

    if (cam_mx == s_last_cam_tx && cam_my == s_last_cam_ty)
        return;

    if (cam_mx != s_last_cam_tx) {
        s32 x = (cam_mx > s_last_cam_tx) ? cam_mx + 8 : cam_mx - 1;
        for (s32 y = cam_my - 1; y <= cam_my + 6; y++)
            write_metatile(x, y);
    }

    if (cam_my != s_last_cam_ty) {
        s32 y = (cam_my > s_last_cam_ty) ? cam_my + 5 : cam_my - 1;
        for (s32 x = cam_mx - 1; x <= cam_mx + 8; x++)
            write_metatile(x, y);
    }

    s_last_cam_tx = cam_mx;
    s_last_cam_ty = cam_my;
}
