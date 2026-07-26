#pragma once

#include "types.h"
#include "party.h"
#include "item.h"
#include "pc.h"

#define SAVE_NAME_LENGTH 8
#define POKEDEX_BYTES 20
#define SAVE_VERSION_CURRENT 5

typedef struct {
    u8  magic[4];
    u8  version;
    u8  map_id;
    u8  last_map_id;
    u8  player_x;
    u8  player_y;
    u8  player_facing;
    u8  player_name[SAVE_NAME_LENGTH];
    u8  rival_name[SAVE_NAME_LENGTH];
    u32 flags[4];
    u8  option_fast_text;
    u8  option_battle_animation;
    u8  option_battle_style;
    PartyState party;
    HealingPoint last_healing_point;
    u32 money;
    BagState bag;
    u8  pokedex_seen[POKEDEX_BYTES];
    u8  pokedex_owned[POKEDEX_BYTES];
    PcState pc;
    u8  checksum;
} SaveData;

bool8 save_exists(void);
bool8 save_read(SaveData *out);
bool8 save_write(const SaveData *data);
SaveData *save_get_buffer(void);
