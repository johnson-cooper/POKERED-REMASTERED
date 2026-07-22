#include "title.h"
#include "gba.h"
#include "text.h"
#include "gfx_title.h"

#define TITLE_CBB       3
#define TITLE_SBB       21
#define TITLE_LOGO_PAL  14
#define TITLE_CHARMANDER_PAL 12
#define TITLE_PLAYER_PAL 11
#define TITLE_BLANK     200
#define TITLE_LOGO_BASE 0
#define TITLE_VERSION_BASE (TITLE_LOGO_BASE + 112)
#define TITLE_PLAYER_BASE  (TITLE_VERSION_BASE + 10)
#define TITLE_CHARMANDER_BASE (TITLE_PLAYER_BASE + 35)
#define INTRO_OAK_BASE (TITLE_CHARMANDER_BASE + 25)
#define INTRO_RIVAL_BASE (INTRO_OAK_BASE + 49)
#define INTRO_RED_BASE (INTRO_RIVAL_BASE + 49)
#define INTRO_NIDORINO_BASE (INTRO_RED_BASE + 49)
#define TITLE_VERSION_PAL 13

#define TITLE_VRAM ((u32 *)(MEM_VRAM + TITLE_CBB * 0x4000))
#define TITLE_SBB_ADDR ((vu16 *)(MEM_VRAM + TITLE_SBB * 0x800))

#define INTRO_TRAINER_PAL 10
#define INTRO_MON_PAL 9

static void copy_title_tiles(u32 tile, const u32 *src, u32 count) {
    u32 *dst = TITLE_VRAM + tile * 8;
    for (u32 i = 0; i < count * 8; i++)
        dst[i] = src[i];
}

static void title_draw_asset(u32 tile, u8 x, u8 y, u8 width, u8 height, u8 palette) {
    for (u8 row = 0; row < height; row++) {
        for (u8 col = 0; col < width; col++) {
            TITLE_SBB_ADDR[(y + row) * 32 + x + col] =
                (u16)(tile + row * width + col | (palette << 12));
        }
    }
}

void title_init(void) {
    copy_title_tiles(TITLE_LOGO_BASE, g_title_logo_tiles, 112);
    copy_title_tiles(TITLE_VERSION_BASE, g_title_version_tiles, 10);
    copy_title_tiles(TITLE_PLAYER_BASE, g_title_player_tiles, 35);
    copy_title_tiles(TITLE_CHARMANDER_BASE, g_title_charmander_tiles, 25);
    copy_title_tiles(INTRO_OAK_BASE, g_intro_oak_tiles, 49);
    copy_title_tiles(INTRO_RIVAL_BASE, g_intro_rival_tiles, 49);
    copy_title_tiles(INTRO_RED_BASE, g_intro_red_tiles, 49);
    copy_title_tiles(INTRO_NIDORINO_BASE, g_intro_nidorino_tiles, 36);

    for (u32 w = 0; w < 8; w++)
        TITLE_VRAM[TITLE_BLANK * 8 + w] = 0;

    // Use a clean white title backdrop.
    PAL_BG[0] = 0x7FFF;

    // Logo: blue-violet outline/shadow with pale yellow lettering.
    vu16 *logo_pal = PAL_BG + TITLE_LOGO_PAL * 16;
    logo_pal[0] = PAL_BG[0];
    logo_pal[1] = (u16)(14 | (14 << 5) | (25 << 10));
    // Medium-gray source pixels are part of the blue outline/shadow, not the
    // yellow lettering. Keeping this entry blue removes the yellow spill.
    logo_pal[2] = (u16)(18 | (18 << 5) | (31 << 10));
    // The light-gray source pixels form the logo lettering.
    logo_pal[3] = (u16)(27 | (24 << 5) | (8 << 10));

    // Charmander: navy outline with warm peach/orange body tones.
    vu16 *charmander_pal = PAL_BG + TITLE_CHARMANDER_PAL * 16;
    charmander_pal[0] = PAL_BG[0];
    charmander_pal[1] = (u16)(8 | (8 << 5) | (16 << 10));
    charmander_pal[2] = (u16)(31 | (17 << 5) | (8 << 10));
    charmander_pal[3] = (u16)(31 | (24 << 5) | (14 << 10));

    // Player: navy outline with muted lavender and warm highlights.
    vu16 *player_pal = PAL_BG + TITLE_PLAYER_PAL * 16;
    player_pal[0] = PAL_BG[0];
    player_pal[1] = (u16)(8 | (8 << 5) | (16 << 10));
    player_pal[2] = (u16)(18 | (15 << 5) | (22 << 10));
    player_pal[3] = (u16)(31 | (24 << 5) | (14 << 10));

    vu16 *trainer_pal = PAL_BG + INTRO_TRAINER_PAL * 16;
    trainer_pal[0] = PAL_BG[0];
    trainer_pal[1] = (u16)(8 | (8 << 5) | (16 << 10));
    trainer_pal[2] = (u16)(18 | (15 << 5) | (22 << 10));
    trainer_pal[3] = (u16)(31 | (24 << 5) | (14 << 10));

    vu16 *mon_pal = PAL_BG + INTRO_MON_PAL * 16;
    mon_pal[0] = PAL_BG[0];
    mon_pal[1] = (u16)(8 | (8 << 5) | (16 << 10));
    mon_pal[2] = (u16)(19 | (16 << 5) | (24 << 10));
    mon_pal[3] = (u16)(31 | (25 << 5) | (25 << 10));

    vu16 *version_pal = PAL_BG + TITLE_VERSION_PAL * 16;
    version_pal[0] = PAL_BG[0];
    version_pal[1] = (u16)(28 | (4 << 5) | (4 << 10));
    version_pal[2] = PAL_BG[0];
}

void title_draw(void) {
    REG_BG1CNT = (u16)(BG_CBB(TITLE_CBB) | BG_SBB(TITLE_SBB) |
                       BG_4BPP | BG_SIZE_256x256 | 1);
    REG_DISPCNT |= DCNT_BG1;

    for (u32 i = 0; i < 32 * 32; i++)
        TITLE_SBB_ADDR[i] = (u16)(TITLE_BLANK | (TITLE_LOGO_PAL << 12));

    // Center the title artwork on the wider color display. The original
    // pokered coordinates assumed a 20-tile Game Boy screen.
    title_draw_asset(TITLE_LOGO_BASE, 7, 1, 16, 7, TITLE_LOGO_PAL);
    title_draw_asset(TITLE_CHARMANDER_BASE, 9, 13, 5, 5, TITLE_CHARMANDER_PAL);
    title_draw_asset(TITLE_PLAYER_BASE, 15, 11, 5, 7, TITLE_PLAYER_PAL);
}

void title_hide(void) {
    REG_DISPCNT &= (u16)~DCNT_BG1;
}

void intro_graphics_show(u8 scene) {
    REG_BG1CNT = (u16)(BG_CBB(TITLE_CBB) | BG_SBB(TITLE_SBB) |
                       BG_4BPP | BG_SIZE_256x256 | 1);
    REG_DISPCNT |= DCNT_BG1;

    for (u32 i = 0; i < 32 * 32; i++)
        TITLE_SBB_ADDR[i] = (u16)(TITLE_BLANK | (TITLE_LOGO_PAL << 12));

    switch (scene) {
    case INTRO_GFX_OAK:
        title_draw_asset(INTRO_OAK_BASE, 11, 3, 7, 7, INTRO_TRAINER_PAL);
        break;
    case INTRO_GFX_NIDORINO:
        title_draw_asset(INTRO_NIDORINO_BASE, 12, 4, 6, 6, INTRO_MON_PAL);
        break;
    case INTRO_GFX_RED:
        title_draw_asset(INTRO_RED_BASE, 11, 3, 7, 7, INTRO_TRAINER_PAL);
        break;
    case INTRO_GFX_RIVAL:
        title_draw_asset(INTRO_RIVAL_BASE, 11, 3, 7, 7, INTRO_TRAINER_PAL);
        break;
    }
}
