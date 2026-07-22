#pragma once
// Pull in the full libgba hardware header set.
// This gives us all register definitions, types, and BIOS calls.
#include <gba_base.h>
#include <gba_video.h>
#include <gba_input.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_dma.h>
#include <gba_sprites.h>
#include "types.h"

// ── Memory map constants (libgba uses MEM_* names) ───────────────────────────
#ifndef MEM_PAL
#define MEM_PAL     0x05000000
#endif
#ifndef MEM_VRAM
#define MEM_VRAM    0x06000000
#endif
#ifndef MEM_OAM
#define MEM_OAM     0x07000000
#endif

// ── Palette pointers ──────────────────────────────────────────────────────────
#define PAL_BG      ((vu16*)MEM_PAL)
#define PAL_OBJ     ((vu16*)(MEM_PAL + 0x200))

// ── Color macro (usable in static const initializers) ─────────────────────────
#define RGB15(r, g, b) ((u16)(((b) << 10) | ((g) << 5) | (r)))

// ── OAM type alias ────────────────────────────────────────────────────────────
// libgba calls this OBJATTR; alias so our code can use ObjAttr
typedef OBJATTR ObjAttr;
// OAM is already defined by gba_sprites.h as ((OBJATTR*)0x07000000)

// ── BG control helpers ────────────────────────────────────────────────────────
#define BG_CBB(n)           ((n) << 2)
#define BG_SBB(n)           ((n) << 8)
#define BG_4BPP             0x0000
#define BG_8BPP             0x0080
#define BG_SIZE_256x256     0x0000
#define BG_SIZE_512x256     0x4000
#define BG_SIZE_256x512     0x8000
#define BG_SIZE_512x512     0xC000

// ── DISPCNT flags ─────────────────────────────────────────────────────────────
#define DCNT_MODE0      MODE_0
#define DCNT_OBJ_MAP_1D OBJ_1D_MAP
#define DCNT_BLANK      LCDC_OFF
#define DCNT_BG0        BG0_ON
#define DCNT_BG1        BG1_ON
#define DCNT_BG2        BG2_ON
#define DCNT_BG3        BG3_ON
#define DCNT_OBJ        OBJ_ON

// ── DMA helpers ───────────────────────────────────────────────────────────────
static inline void dma_copy32(void *dst, const void *src, u32 words) {
    DMA3COPY(src, dst, DMA32 | DMA_ENABLE | words);
}

static inline void dma_fill32(void *dst, u32 value, u32 words) {
    static u32 _v;
    _v = value;
    DMA3COPY(&_v, dst, DMA32 | DMA_ENABLE | DMA_SRC_FIXED | words);
}
