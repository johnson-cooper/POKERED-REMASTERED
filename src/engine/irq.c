#include "irq.h"
#include "gba.h"
#include <gba_interrupt.h>

volatile u32 g_vblank_count = 0;

static void vblank_counter(void) {
    g_vblank_count++;
}

void irq_init(void) {
    irqInit();                         // libgba: installs BIOS-compatible master handler
    irqSet(IRQ_VBLANK, vblank_counter);
    irqEnable(IRQ_VBLANK);
    REG_IME = 1;
}

void irq_set(u16 irq_flag, IrqHandler handler) {
    irqSet((irqMASK)irq_flag, handler);
    irqEnable((irqMASK)irq_flag);
}
