#include "render.h"
#include "gba.h"

// ── Command queue ─────────────────────────────────────────────────────────────
static RenderCmd s_queue[RENDER_QUEUE_SIZE];
static u32       s_queue_head = 0;
static u32       s_queue_tail = 0;

// ── OAM shadow (write here during logic, DMA to real OAM at VBlank) ──────────
static OBJATTR s_oam_shadow[128];
static u32     s_oam_index = 0;

void render_init(void) {
    // Minimal init: blank screen until tilemap_init() sets final DISPCNT.
    // BG layers are enabled by tilemap_init() when the world loads.
    REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ_MAP_1D;

    // Clear OAM by hiding all 128 sprites
    for (u32 i = 0; i < 128; i++) {
        OAM[i].attr0 = 0x0200; // OBJ disable
        OAM[i].attr1 = 0;
        OAM[i].attr2 = 0;
    }

    // Clear palette RAM
    dma_fill32((void*)MEM_PAL, 0, 0x200 / 4);

    s_queue_head = 0;
    s_queue_tail = 0;
    s_oam_index  = 0;
}

void render_submit(RenderCmd cmd) {
    u32 next = (s_queue_tail + 1) % RENDER_QUEUE_SIZE;
    if (next != s_queue_head) {
        s_queue[s_queue_tail] = cmd;
        s_queue_tail = next;
    }
}

static void process_cmd(const RenderCmd *cmd) {
    switch (cmd->type) {
    case RCMD_CLEAR_SPRITES:
        s_oam_index = 0;
        break;
    case RCMD_DRAW_SPRITE:
        if (s_oam_index < 128) {
            // 16×16 sprite, 4bpp; bit 4 selects the NPC palette bank.
            // attr0 bits 14-15 = shape 00 (square) — no extra bits needed
            // attr1 bits 14-15 = size  01 (16px) = 0x4000
            s_oam_shadow[s_oam_index].attr0 = (u16)(cmd->y & 0xFF);
            s_oam_shadow[s_oam_index].attr1 = (u16)(cmd->x & 0x1FF) | 0x4000;
            if (cmd->param & 1)
                s_oam_shadow[s_oam_index].attr1 |= 0x1000; // horizontal flip
            // Keep world sprites below BG0 so dialog/name windows remain
            // opaque and always cover NPCs and the player.
            s_oam_shadow[s_oam_index].attr2 =
                (u16)(cmd->id | (1 << 10) |
                      (((cmd->param >> 4) & 0xF) << 12));
            s_oam_index++;
        }
        break;
    case RCMD_DRAW_BG_TILE: {
        vu16 *sbb = (vu16*)(MEM_VRAM + cmd->id * 0x800);
        sbb[(cmd->y & 31) * 32 + (cmd->x & 31)] = cmd->param;
        break;
    }
    case RCMD_SCROLL_BG:
        switch (cmd->id) {
        case 0: REG_BG0HOFS = (u16)cmd->x; REG_BG0VOFS = (u16)cmd->y; break;
        case 1: REG_BG1HOFS = (u16)cmd->x; REG_BG1VOFS = (u16)cmd->y; break;
        case 2: REG_BG2HOFS = (u16)cmd->x; REG_BG2VOFS = (u16)cmd->y; break;
        case 3: REG_BG3HOFS = (u16)cmd->x; REG_BG3VOFS = (u16)cmd->y; break;
        }
        break;
    case RCMD_DRAW_SPRITE_LARGE:
        if (s_oam_index < 128) {
            s_oam_shadow[s_oam_index].attr0 = (u16)(cmd->y & 0xFF);
            s_oam_shadow[s_oam_index].attr1 = (u16)((cmd->x & 0x1FF) | 0xC000);
            if (cmd->param & 0x10)
                s_oam_shadow[s_oam_index].attr1 |= 0x1000;
            s_oam_shadow[s_oam_index].attr2 =
                (u16)(cmd->id | ((cmd->param & 0xF) << 12));
            s_oam_index++;
        }
        break;
    default:
        break;
    }
}

// Called from VBlank IRQ — drain the command queue and DMA OAM shadow.
// oam_index resets here so sprites submitted during game_update are visible
// this frame, not wiped by a stale clear command.
void render_flush(void) {
    s_oam_index = 0;  // start fresh each VBlank

    while (s_queue_head != s_queue_tail) {
        process_cmd(&s_queue[s_queue_head]);
        s_queue_head = (s_queue_head + 1) % RENDER_QUEUE_SIZE;
    }
    // Disable unused OAM slots
    for (u32 i = s_oam_index; i < 128; i++) {
        s_oam_shadow[i].attr0 = 0x0200;
        s_oam_shadow[i].attr1 = 0;
        s_oam_shadow[i].attr2 = 0;
    }
    dma_copy32(OAM, s_oam_shadow, sizeof(s_oam_shadow) / 4);
}

void render_clear_sprites(void) {
    RenderCmd cmd = { .type = RCMD_CLEAR_SPRITES };
    render_submit(cmd);
}
