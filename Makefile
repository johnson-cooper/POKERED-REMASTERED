# ── Pokémon Red GBA Remaster ──────────────────────────────────────────────────

TARGET  := pokered_remaster
BUILD   := build
SRCDIRS := src/engine src/graphics src/input src/audio src/battle \
           src/pokemon src/world src/ui src/data src/save

# GCC needs a writable temporary directory. Some Windows/MSYS2 environments
# inherit an unwritable C:\WINDOWS as TEMP. Force all three variables to the
# build directory so compilation and linking always succeed.
override TEMP   := $(CURDIR)/$(BUILD)
override TMP    := $(CURDIR)/$(BUILD)
override TMPDIR := $(CURDIR)/$(BUILD)
export TEMP TMP TMPDIR

# ── devkitPro / devkitARM ─────────────────────────────────────────────────────
ifeq ($(strip $(DEVKITPRO)),)
  DEVKITPRO := /c/devkitpro
endif
export DEVKITPRO

ifeq ($(strip $(DEVKITARM)),)
  DEVKITARM := $(DEVKITPRO)/devkitARM
endif
export DEVKITARM

include $(DEVKITARM)/base_rules

LIBGBA   := $(DEVKITPRO)/libgba
LIBGBAINC := $(LIBGBA)/include
LIBGBALIB := $(LIBGBA)/lib

# ── Toolchain ─────────────────────────────────────────────────────────────────
PREFIX  := arm-none-eabi-
CC      := $(PREFIX)gcc
LD      := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
GBAFIX  := $(DEVKITPRO)/tools/bin/gbafix

# ── Flags ─────────────────────────────────────────────────────────────────────
ARCH    := -mthumb -mthumb-interwork -mcpu=arm7tdmi -mtune=arm7tdmi
CFLAGS  := $(ARCH) -pipe \
            -O2 \
            -Wall -Wextra -Wno-unused-parameter \
            -ffunction-sections -fdata-sections \
            -Iinclude \
            -Isrc/data \
            -I$(LIBGBAINC)

LDFLAGS := $(ARCH) \
            -specs=$(DEVKITARM)/arm-none-eabi/lib/gba.specs \
            -L$(LIBGBALIB) \
            -Wl,--gc-sections \
            -Wl,-Map,$(BUILD)/$(TARGET).map

LIBS    := -lgba

# ── Source discovery ──────────────────────────────────────────────────────────
CFILES  := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
SFILES  := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.s))
OFILES  := $(patsubst %.c,$(BUILD)/%.o,$(CFILES)) \
           $(patsubst %.s,$(BUILD)/%.o,$(SFILES))

DFILES  := $(OFILES:.o=.d)
-include $(DFILES)

# ── Targets ───────────────────────────────────────────────────────────────────
.PHONY: all clean

all: $(TARGET).gba

$(TARGET).gba: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	$(GBAFIX) $@ -t"Pokemon Red Remaster" -c"BPRE" -m96
	@echo ""
	@echo "  Built: $@"
	@ls -lh $@

$(TARGET).elf: $(OFILES)
	TEMP="$(CURDIR)/$(BUILD)" TMP="$(CURDIR)/$(BUILD)" TMPDIR="$(CURDIR)/$(BUILD)" \
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -x assembler-with-cpp -MMD -MP -c -o $@ $<

clean:
	rm -rf $(BUILD) $(TARGET).gba $(TARGET).elf
