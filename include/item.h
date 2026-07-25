#pragma once
#include "types.h"

typedef enum {
    ITEM_NONE = 0,
    ITEM_POKE_BALL,
    ITEM_POTION,
    ITEM_ANTIDOTE,
    ITEM_PARLYZ_HEAL,
    ITEM_BURN_HEAL,
    ITEM_OAKS_PARCEL,
    ITEM_TOWN_MAP,
    ITEM_COUNT,
} ItemId;

#define BAG_MAX_SLOTS  20
#define BAG_MAX_STACK  99

typedef struct {
    u8 id;
    u8 quantity;
} BagSlot;

typedef struct {
    u8 count;
    BagSlot slots[BAG_MAX_SLOTS];
} BagState;

extern BagState g_bag;

const char *item_get_name(ItemId id);
u16         item_get_price(ItemId id);
bool8       item_is_key_item(ItemId id);
void        bag_init(void);
bool8       bag_add(ItemId id, u8 quantity);
bool8       bag_remove(ItemId id, u8 quantity);
u8          bag_count(ItemId id);
void        bag_export(BagState *out);
void        bag_import(const BagState *in);
