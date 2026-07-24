#pragma once
#include "types.h"

void title_init(void);
void title_draw(void);
void title_hide(void);

#define INTRO_GFX_OAK       0
#define INTRO_GFX_NIDORINO  1
#define INTRO_GFX_RED       2
#define INTRO_GFX_RIVAL     3
#define INTRO_GFX_SHRINK1   4
#define INTRO_GFX_SHRINK2   5

void intro_graphics_show(u8 scene);
