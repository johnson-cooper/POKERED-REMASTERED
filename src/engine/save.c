#include "save.h"
#include "experience.h"
#include <stddef.h>

#define SAVE_VERSION 3
#define SAVE_MAGIC_0 'R'
#define SAVE_MAGIC_1 'R'
#define SAVE_MAGIC_2 'M'
#define SAVE_MAGIC_3 '1'
#define GBA_SRAM ((volatile u8 *)0x0E000000)
#define GBA_WAITCNT (*(volatile u16 *)0x04000204)

// SRAM is an 8-bit cartridge bus. Use the slowest SRAM timing so this works
// reliably on hardware and on emulators/flashcarts with stricter bus timing.
static void save_prepare_sram(void) {
    GBA_WAITCNT = (u16)((GBA_WAITCNT & (u16)~3) | 3);
}

// mGBA and several other emulators use the conventional ROM marker to select
// the cartridge save backend. The physical GBA ignores this string, while
// emulators use it to map 32 KiB of SRAM at 0x0E000000.
static volatile const char s_save_type_marker[] = "SRAM_V113";

static u8 save_checksum(const SaveData *data) {
    const u8 *bytes = (const u8 *)data;
    u8 sum = 0;
    for (u32 i = 4; i < offsetof(SaveData, checksum); i++)
        sum = (u8)(sum + bytes[i]);
    return sum;
}

typedef struct {
    u8 magic[4];
    u8 version;
    u8 map_id;
    u8 last_map_id;
    u8 player_x;
    u8 player_y;
    u8 player_facing;
    u8 player_name[SAVE_NAME_LENGTH];
    u8 rival_name[SAVE_NAME_LENGTH];
    u32 flags[4];
    u8 option_fast_text;
    u8 option_battle_animation;
    u8 option_battle_style;
    u8 checksum;
} LegacySaveData;

// Version 2 included party data but predates the persisted experience field.
// Keep its exact layout so existing saves can be migrated safely.
typedef struct {
    PokemonId species;
    u8 level;
    u16 dv;
    u16 max_hp, current_hp;
    u16 attack, defense, speed, special;
    MoveId moves[4];
    u8 pp[4];
    u8 status;
    char nickname[PARTY_NICKNAME_LENGTH];
} LegacyPartyPokemon;

typedef struct {
    u8 count;
    LegacyPartyPokemon mons[PARTY_SIZE];
} LegacyPartyState;

typedef struct {
    u8 magic[4];
    u8 version;
    u8 map_id;
    u8 last_map_id;
    u8 player_x;
    u8 player_y;
    u8 player_facing;
    u8 player_name[SAVE_NAME_LENGTH];
    u8 rival_name[SAVE_NAME_LENGTH];
    u32 flags[4];
    u8 option_fast_text;
    u8 option_battle_animation;
    u8 option_battle_style;
    LegacyPartyState party;
    HealingPoint last_healing_point;
    u8 checksum;
} LegacyV2SaveData;

static u8 legacy_checksum(const LegacySaveData *data) {
    const u8 *bytes = (const u8 *)data;
    u8 sum = 0;
    for (u32 i = 4; i < offsetof(LegacySaveData, checksum); i++)
        sum = (u8)(sum + bytes[i]);
    return sum;
}

static u8 legacy_v2_checksum(const LegacyV2SaveData *data) {
    const u8 *bytes = (const u8 *)data;
    u8 sum = 0;
    for (u32 i = 4; i < offsetof(LegacyV2SaveData, checksum); i++)
        sum = (u8)(sum + bytes[i]);
    return sum;
}

static bool8 save_header_valid(void) {
    if (s_save_type_marker[0] != 'S' || s_save_type_marker[8] != '3')
        return FALSE;
    return GBA_SRAM[0] == SAVE_MAGIC_0 &&
           GBA_SRAM[1] == SAVE_MAGIC_1 &&
           GBA_SRAM[2] == SAVE_MAGIC_2 &&
           GBA_SRAM[3] == SAVE_MAGIC_3 &&
           (GBA_SRAM[4] == 1 || GBA_SRAM[4] == 2 || GBA_SRAM[4] == SAVE_VERSION);
}

bool8 save_exists(void) {
    save_prepare_sram();
    if (!save_header_valid()) return FALSE;

    if (GBA_SRAM[4] == 1) {
        LegacySaveData data;
        for (u32 i = 0; i < sizeof(LegacySaveData); i++)
            ((u8 *)&data)[i] = GBA_SRAM[i];
        return data.checksum == legacy_checksum(&data);
    }
    if (GBA_SRAM[4] == 2) {
        LegacyV2SaveData data;
        for (u32 i = 0; i < sizeof(LegacyV2SaveData); i++)
            ((u8 *)&data)[i] = GBA_SRAM[i];
        return data.checksum == legacy_v2_checksum(&data);
    }
    SaveData data;
    for (u32 i = 0; i < sizeof(SaveData); i++)
        ((u8 *)&data)[i] = GBA_SRAM[i];
    return data.checksum == save_checksum(&data);
}

bool8 save_read(SaveData *out) {
    save_prepare_sram();
    if (!out || !save_exists()) return FALSE;
    for (u32 i = 0; i < sizeof(SaveData); i++)
        ((u8 *)out)[i] = 0;
    if (GBA_SRAM[4] == 1) {
        LegacySaveData legacy;
        for (u32 i = 0; i < sizeof(LegacySaveData); i++)
            ((u8 *)&legacy)[i] = GBA_SRAM[i];
        out->magic[0] = legacy.magic[0];
        out->magic[1] = legacy.magic[1];
        out->magic[2] = legacy.magic[2];
        out->magic[3] = legacy.magic[3];
        out->version = legacy.version;
        out->map_id = legacy.map_id;
        out->last_map_id = legacy.last_map_id;
        out->player_x = legacy.player_x;
        out->player_y = legacy.player_y;
        out->player_facing = legacy.player_facing;
        for (u8 i = 0; i < SAVE_NAME_LENGTH; i++) {
            out->player_name[i] = legacy.player_name[i];
            out->rival_name[i] = legacy.rival_name[i];
        }
        for (u8 i = 0; i < 4; i++) out->flags[i] = legacy.flags[i];
        out->option_fast_text = legacy.option_fast_text;
        out->option_battle_animation = legacy.option_battle_animation;
        out->option_battle_style = legacy.option_battle_style;
        out->checksum = legacy.checksum;
    } else if (GBA_SRAM[4] == 2) {
        LegacyV2SaveData legacy;
        for (u32 i = 0; i < sizeof(LegacyV2SaveData); i++)
            ((u8 *)&legacy)[i] = GBA_SRAM[i];
        out->magic[0] = legacy.magic[0];
        out->magic[1] = legacy.magic[1];
        out->magic[2] = legacy.magic[2];
        out->magic[3] = legacy.magic[3];
        out->version = legacy.version;
        out->map_id = legacy.map_id;
        out->last_map_id = legacy.last_map_id;
        out->player_x = legacy.player_x;
        out->player_y = legacy.player_y;
        out->player_facing = legacy.player_facing;
        for (u8 i = 0; i < SAVE_NAME_LENGTH; i++) {
            out->player_name[i] = legacy.player_name[i];
            out->rival_name[i] = legacy.rival_name[i];
        }
        for (u8 i = 0; i < 4; i++) out->flags[i] = legacy.flags[i];
        out->option_fast_text = legacy.option_fast_text;
        out->option_battle_animation = legacy.option_battle_animation;
        out->option_battle_style = legacy.option_battle_style;
        out->party.count = legacy.party.count;
        for (u8 i = 0; i < PARTY_SIZE; i++) {
            const LegacyPartyPokemon *src = &legacy.party.mons[i];
            PartyPokemon *dst = &out->party.mons[i];
            dst->species = src->species;
            dst->level = src->level;
            dst->dv = src->dv;
            dst->max_hp = src->max_hp;
            dst->current_hp = src->current_hp;
            dst->attack = src->attack;
            dst->defense = src->defense;
            dst->speed = src->speed;
            dst->special = src->special;
            for (u8 j = 0; j < 4; j++) {
                dst->moves[j] = src->moves[j];
                dst->pp[j] = src->pp[j];
            }
            dst->status = src->status;
            for (u8 j = 0; j < PARTY_NICKNAME_LENGTH; j++)
                dst->nickname[j] = src->nickname[j];
            dst->experience = pokemon_exp_for_level(dst->species, dst->level);
        }
        out->last_healing_point = legacy.last_healing_point;
        out->checksum = legacy.checksum;
    } else {
        for (u32 i = 0; i < sizeof(SaveData); i++)
            ((u8 *)out)[i] = GBA_SRAM[i];
    }
    return TRUE;
}

bool8 save_write(const SaveData *data) {
    if (!data) return FALSE;

    save_prepare_sram();

    SaveData copy = *data;
    copy.magic[0] = SAVE_MAGIC_0;
    copy.magic[1] = SAVE_MAGIC_1;
    copy.magic[2] = SAVE_MAGIC_2;
    copy.magic[3] = SAVE_MAGIC_3;
    copy.version = SAVE_VERSION;
    copy.checksum = save_checksum(&copy);

    // Write the payload first and the magic last. A reset during saving will
    // therefore leave the old file recognizable instead of a false new file.
    for (u32 i = 4; i < sizeof(SaveData); i++)
        GBA_SRAM[i] = ((const u8 *)&copy)[i];
    GBA_SRAM[0] = copy.magic[0];
    GBA_SRAM[1] = copy.magic[1];
    GBA_SRAM[2] = copy.magic[2];
    GBA_SRAM[3] = copy.magic[3];
    return save_exists();
}
