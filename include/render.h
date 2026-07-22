#pragma once
#include "types.h"

// Abstract render command queue — game logic posts commands here,
// the GBA renderer consumes them each VBlank.
// This decouples game logic from hardware so other renderers can be swapped in.

typedef enum {
    RCMD_NONE = 0,
    RCMD_DRAW_SPRITE,
    RCMD_DRAW_BG_TILE,
    RCMD_SCROLL_BG,
    RCMD_LOAD_PALETTE,
    RCMD_CLEAR_SPRITES,
    RCMD_DRAW_SPRITE_LARGE,
} RenderCmdType;

typedef struct {
    RenderCmdType type;
    u16 id;
    s16 x, y;
    u16 param;
} RenderCmd;

#define RENDER_QUEUE_SIZE 128

void render_init(void);
void render_flush(void);          // called by VBlank handler
void render_submit(RenderCmd cmd);
void render_clear_sprites(void);
