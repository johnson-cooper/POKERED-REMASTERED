#include "gba.h"
#include "irq.h"
#include "render.h"
#include "game.h"
#include "text.h"
#include "title.h"
#include <gba_interrupt.h>
#include <gba_systemcalls.h>

static void on_vblank(void) {
    render_flush();
    g_vblank_count++;
}

int main(void) {
    irqInit();
    irqSet(IRQ_VBLANK, on_vblank);
    irqEnable(IRQ_VBLANK);

    render_init();
    // Load the font and its palette before the title state starts drawing.
    text_init();
    title_init();
    game_init();

    while (1) {
        VBlankIntrWait();   // libgba BIOS call — correctly notifies BIOS_IF
        game_update();
    }

    return 0;
}
