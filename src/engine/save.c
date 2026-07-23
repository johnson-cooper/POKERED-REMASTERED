#include "save.h"

#define SAVE_VERSION 1
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
    for (u32 i = 4; i < sizeof(SaveData) - 1; i++)
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
           GBA_SRAM[4] == SAVE_VERSION;
}

bool8 save_exists(void) {
    save_prepare_sram();
    if (!save_header_valid()) return FALSE;

    SaveData data;
    for (u32 i = 0; i < sizeof(SaveData); i++)
        ((u8 *)&data)[i] = GBA_SRAM[i];
    return data.checksum == save_checksum(&data);
}

bool8 save_read(SaveData *out) {
    save_prepare_sram();
    if (!out || !save_exists()) return FALSE;
    for (u32 i = 0; i < sizeof(SaveData); i++)
        ((u8 *)out)[i] = GBA_SRAM[i];
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
