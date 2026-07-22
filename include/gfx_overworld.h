#pragma once
#include "types.h"

extern const u32 g_overworld_tiles[];
extern const u32 g_overworld_tile_count;

// Runtime-generated overlay copies live immediately after the original pokered
// overworld tiles in VRAM.
#define OVERWORLD_OVERLAY_TRANSPARENT_WHITE_BASE 96
#define OVERWORLD_OVERLAY_EDGE_MASK_BASE        192
#define OVERWORLD_OVERLAY_SOLID_BASE            288
