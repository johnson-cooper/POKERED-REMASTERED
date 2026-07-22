#include "input.h"

u16 g_keys_held     = 0;
u16 g_keys_pressed  = 0;
u16 g_keys_released = 0;

// GBA key register uses active-low logic: bit=0 means pressed.
// We invert so that bit=1 means pressed, matching intuitive usage.
void input_update(void) {
    u16 raw   = ~REG_KEYINPUT & 0x03FF;
    u16 prev  = g_keys_held;
    g_keys_held     = raw;
    g_keys_pressed  = raw & ~prev;
    g_keys_released = ~raw & prev;
}
