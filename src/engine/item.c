#include "item.h"

BagState g_bag;

typedef struct {
    const char *name;
    u16 price;
    bool8 key_item;
} ItemDef;

static const ItemDef s_item_defs[ITEM_COUNT] = {
    [ITEM_NONE]        = { "?????",       0,   FALSE },
    [ITEM_POKE_BALL]   = { "POKe BALL",   200, FALSE },
    [ITEM_POTION]      = { "POTION",      300, FALSE },
    [ITEM_ANTIDOTE]    = { "ANTIDOTE",    100, FALSE },
    [ITEM_PARLYZ_HEAL] = { "PARLYZ HEAL", 200, FALSE },
    [ITEM_BURN_HEAL]   = { "BURN HEAL",   250, FALSE },
    [ITEM_OAKS_PARCEL] = { "OAK's PARCEL", 0,  TRUE  },
    [ITEM_TOWN_MAP]    = { "TOWN MAP",     0,  TRUE  },
    [ITEM_FIRE_STONE]  = { "FIRE STONE",   2100, FALSE },
    [ITEM_THUNDER_STONE] = { "THUNDERSTONE", 2100, FALSE },
    [ITEM_WATER_STONE] = { "WATER STONE",  2100, FALSE },
    [ITEM_LEAF_STONE]  = { "LEAF STONE",   2100, FALSE },
    [ITEM_MOON_STONE]  = { "MOON STONE",   2100, FALSE },
};

const char *item_get_name(ItemId id) {
    static char machine_name[6];
    if (id >= ITEM_TM01 && id <= ITEM_HM05) {
        u8 number = (u8)(id - ITEM_TM01);
        machine_name[0] = number < 50 ? 'T' : 'H';
        machine_name[1] = number < 50 ? 'M' : 'M';
        number = number < 50 ? (u8)(number + 1) : (u8)(number - 49);
        machine_name[2] = (char)('0' + number / 10);
        machine_name[3] = (char)('0' + number % 10);
        machine_name[4] = '\0';
        return machine_name;
    }
    if (id >= ITEM_COUNT) return "?????";
    return s_item_defs[id].name;
}

u16 item_get_price(ItemId id) {
    if (id >= ITEM_COUNT) return 0;
    return s_item_defs[id].price;
}

bool8 item_is_key_item(ItemId id) {
    if (id >= ITEM_COUNT) return FALSE;
    return s_item_defs[id].key_item;
}

bool8 item_is_tmhm(ItemId id) {
    return id >= ITEM_TM01 && id <= ITEM_HM05;
}

u8 item_tmhm_number(ItemId id) {
    if (!item_is_tmhm(id)) return 0;
    return (u8)(id - ITEM_TM01 + 1);
}

void bag_init(void) {
    g_bag.count = 0;
    for (u8 i = 0; i < BAG_MAX_SLOTS; i++) {
        g_bag.slots[i].id = ITEM_NONE;
        g_bag.slots[i].quantity = 0;
    }
}

bool8 bag_add(ItemId id, u8 quantity) {
    if (id == ITEM_NONE || id >= ITEM_COUNT || quantity == 0) return FALSE;
    for (u8 i = 0; i < g_bag.count; i++) {
        if (g_bag.slots[i].id == (u8)id) {
            u16 total = (u16)g_bag.slots[i].quantity + quantity;
            if (total > BAG_MAX_STACK) return FALSE;
            g_bag.slots[i].quantity = (u8)total;
            return TRUE;
        }
    }
    if (g_bag.count >= BAG_MAX_SLOTS) return FALSE;
    g_bag.slots[g_bag.count].id = (u8)id;
    g_bag.slots[g_bag.count].quantity = quantity;
    g_bag.count++;
    return TRUE;
}

bool8 bag_remove(ItemId id, u8 quantity) {
    if (id == ITEM_NONE || id >= ITEM_COUNT || quantity == 0) return FALSE;
    for (u8 i = 0; i < g_bag.count; i++) {
        if (g_bag.slots[i].id == (u8)id) {
            if (g_bag.slots[i].quantity < quantity) return FALSE;
            g_bag.slots[i].quantity -= quantity;
            if (g_bag.slots[i].quantity == 0) {
                for (u8 j = i; j + 1 < g_bag.count; j++)
                    g_bag.slots[j] = g_bag.slots[j + 1];
                g_bag.count--;
                g_bag.slots[g_bag.count].id = ITEM_NONE;
                g_bag.slots[g_bag.count].quantity = 0;
            }
            return TRUE;
        }
    }
    return FALSE;
}

u8 bag_count(ItemId id) {
    for (u8 i = 0; i < g_bag.count; i++) {
        if (g_bag.slots[i].id == (u8)id)
            return g_bag.slots[i].quantity;
    }
    return 0;
}

void bag_export(BagState *out) {
    *out = g_bag;
}

void bag_import(const BagState *in) {
    g_bag = *in;
}
