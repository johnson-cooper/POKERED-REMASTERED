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
    ITEM_FIRE_STONE,
    ITEM_THUNDER_STONE,
    ITEM_WATER_STONE,
    ITEM_LEAF_STONE,
    ITEM_MOON_STONE,
    ITEM_TM01,
    ITEM_TM02, ITEM_TM03, ITEM_TM04, ITEM_TM05, ITEM_TM06, ITEM_TM07, ITEM_TM08, ITEM_TM09, ITEM_TM10,
    ITEM_TM11, ITEM_TM12, ITEM_TM13, ITEM_TM14, ITEM_TM15, ITEM_TM16, ITEM_TM17, ITEM_TM18, ITEM_TM19, ITEM_TM20,
    ITEM_TM21, ITEM_TM22, ITEM_TM23, ITEM_TM24, ITEM_TM25, ITEM_TM26, ITEM_TM27, ITEM_TM28, ITEM_TM29, ITEM_TM30,
    ITEM_TM31, ITEM_TM32, ITEM_TM33, ITEM_TM34, ITEM_TM35, ITEM_TM36, ITEM_TM37, ITEM_TM38, ITEM_TM39, ITEM_TM40,
    ITEM_TM41, ITEM_TM42, ITEM_TM43, ITEM_TM44, ITEM_TM45, ITEM_TM46, ITEM_TM47, ITEM_TM48, ITEM_TM49, ITEM_TM50,
    ITEM_HM01, ITEM_HM02, ITEM_HM03, ITEM_HM04, ITEM_HM05,
    ITEM_HP_UP,
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
bool8       item_is_tmhm(ItemId id);
u8          item_tmhm_number(ItemId id);
void        bag_init(void);
bool8       bag_add(ItemId id, u8 quantity);
bool8       bag_remove(ItemId id, u8 quantity);
u8          bag_count(ItemId id);
void        bag_export(BagState *out);
void        bag_import(const BagState *in);
