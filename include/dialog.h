#pragma once
#include "types.h"

// Dialog box — bordered text box drawn on BG0, bottom 4 rows.
// Caller feeds text; dialog_update() returns TRUE when text is fully consumed
// and the box is closed (player pressed A on the final page).

void  dialog_open(void);
void  dialog_set_text(const char *s);  // set current message (may span pages)
bool8 dialog_update(void);             // call each frame; TRUE when closed
bool8 dialog_is_open(void);
void  dialog_close(void);
void  dialog_yesno_close(void);

// YES/NO menu drawn above the dialog box. Returns 0xFF while pending,
// 1 for YES, 0 for NO.
void dialog_yesno_open(void);
u8   dialog_yesno_update(void);
