#pragma once
#include "types.h"

typedef void (*IrqHandler)(void);

// irq_init: set up libgba interrupt system + VBlank counter
void irq_init(void);

// irq_set: register a handler for the given IRQ_* flag (from gba_interrupt.h)
void irq_set(u16 irq_flag, IrqHandler handler);

extern volatile u32 g_vblank_count;
