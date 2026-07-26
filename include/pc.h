#pragma once

#include "types.h"
#include "party.h"
#include "item.h"

#define PC_BOX_SIZE     20
#define PC_NUM_BOXES    12
#define PC_ITEM_SLOTS   50

typedef struct {
    u8 count;
    PartyPokemon mons[PC_BOX_SIZE];
} PcBox;

typedef struct {
    u8 current_box;
    PcBox boxes[PC_NUM_BOXES];
    u8 item_count;
    BagSlot items[PC_ITEM_SLOTS];
} PcState;

extern PcState g_pc;

void  pc_init(void);
bool8 pc_deposit_pokemon(u8 party_slot);
bool8 pc_withdraw_pokemon(u8 box_slot);
bool8 pc_box_has_space(void);
u8    pc_current_box_count(void);
void  pc_change_box(u8 box);
bool8 pc_deposit_item(ItemId id, u8 quantity);
bool8 pc_withdraw_item(ItemId id, u8 quantity);
u8    pc_item_count(ItemId id);

// PC menu (called from overworld script when interacting with a PC tile)
void  pc_menu_open(void);
bool8 pc_menu_update(void);
void  pc_menu_close(void);
bool8 pc_menu_is_open(void);

// Save/load
void  pc_export(PcState *out);
void  pc_import(const PcState *in);
