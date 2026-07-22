#pragma once
#include "types.h"
#include "gba.h"

void input_update(void);

// Returns non-zero if the key is currently held
static inline bool8 input_held(u16 key)   { extern u16 g_keys_held;    return (g_keys_held    & key) != 0; }
// Returns non-zero on the first frame the key was pressed
static inline bool8 input_pressed(u16 key){ extern u16 g_keys_pressed; return (g_keys_pressed & key) != 0; }
// Returns non-zero on the first frame the key was released
static inline bool8 input_released(u16 key){ extern u16 g_keys_released; return (g_keys_released & key) != 0; }
