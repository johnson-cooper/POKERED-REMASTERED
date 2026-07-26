# Pokémon Red GBA Remaster — Architecture Reference

This document describes how the entire codebase is structured, how each system
works, and how the systems connect to each other. It is meant as a complete
mental model for any future development session.

---

## 1. Platform & Build

| Attribute | Value |
|-----------|-------|
| Target CPU | ARM7TDMI (GBA) — Thumb mode |
| Toolchain | devkitARM (arm-none-eabi-gcc), libgba |
| Output | `pokered_remaster.gba` |
| Optimization | `-O2 -flto` in both CFLAGS and LDFLAGS |
| Source tree | `src/` + `include/` |
| Reference repos | `refs/pokered/`, `refs/pokeemerald/`, `refs/pokefirered/` |

Build commands:
```
make          # compile + link + fix GBA header
make patcher  # generate patcher/index.html (bumps patcher/VERSION)
make clean
```

The Makefile auto-discovers every `.c` and `.s` under the directories listed in
`SRCDIRS`. The GBA binary is fixed up by `gbafix` with code `BPRE`.

---

## 2. Entry Point & Main Loop (`src/engine/main.c`)

```
main()
  irqInit() / irqSet(IRQ_VBLANK, on_vblank) / irqEnable(IRQ_VBLANK)
  render_init()
  audio_init()
  text_init()
  title_init()
  game_init()
  loop:
    VBlankIntrWait()   ← BIOS call, blocks until VBlank fires
    game_update()
```

**VBlank ISR** (`on_vblank`): runs every ~16.7 ms.
- `audio_update()` — advances tracker playback
- `render_flush()` — writes queued sprite/palette commands to OAM/VRAM
- increments `g_vblank_count` (used for animation timers and NPC movement)

`game_update()` runs once per VBlank immediately after the wait returns. It
reads input, advances the current game state, and increments `g_game.frame`.

---

## 3. Game State Machine (`include/game.h`, `src/engine/game.c`)

```c
typedef enum {
    GAME_STATE_BOOT,
    GAME_STATE_TITLE,
    GAME_STATE_INTRO,
    GAME_STATE_OVERWORLD,
    GAME_STATE_BATTLE,
    GAME_STATE_MENU,
} GameState;
```

`game_update()` dispatches to a function table indexed by the current state.
State transitions happen via `game_change_state(new_state)`, which sets
`g_game.next_state`. The transition fires at the top of the *next* frame so the
current tick finishes cleanly.

### On-enter side-effects (inside `game_update`)

| Entering state | What happens |
|---------------|--------------|
| `TITLE` | `title_draw()`, "PRESS START" text, plays title music |
| `INTRO` | shows Oak intro graphic, begins Oak's dialog, resets naming state |
| `OVERWORLD` | if `s_continue_load` → `game_load_saved()`, otherwise fresh `world_init()` into Red's House 2F; if coming from BATTLE → resume map music |
| `BATTLE` | `battle_transition_start()`, plays wild/trainer music |
| `MENU` | resets all pause-menu sub-screen flags, calls `pause_menu_draw()` |

### Global game variables

| Variable | Type | Description |
|----------|------|-------------|
| `g_game` | `GameContext` | current/next state + frame counter |
| `g_player_money` | `u32` | capped at 999,999 |

---

## 4. Title Screen

Three sub-modes live inside `GAME_STATE_TITLE`:

```
TITLE_MODE_PRESS_START  → blinks "PRESS START", waits for START/A
TITLE_MODE_MAIN_MENU    → CONTINUE / NEW GAME / OPTION (CONTINUE only if save exists)
TITLE_MODE_OPTIONS      → TEXT SPEED / BATTLE ANIM / BATTLE STYLE / CANCEL
```

Options are stored in static booleans (`s_option_fast_text`, etc.) and saved to
SRAM inside `SaveData`. They can also be opened from the pause menu
(`s_options_from_pause = TRUE`).

---

## 5. Intro Sequence

State machine: `IntroState` (11 states in `src/engine/game.c`).

```
INTRO_OAK_DIALOG        Oak says "Hello there!"
INTRO_NIDORINO_DIALOG   Oak explains Pokémon world
INTRO_PLAYER_DIALOG     "First, what is your name?"
INTRO_PLAYER_NAME_SELECT  6-row × 9-col character grid
INTRO_PLAYER_NAME_CONFIRM Oak confirms name
INTRO_RIVAL_INTRO       Oak introduces rival
INTRO_RIVAL_NAME_SELECT   same grid for rival
INTRO_RIVAL_NAME_CONFIRM  Oak confirms rival name
INTRO_DONE              "Your very own POKéMON legend..."
INTRO_PLAYER_SHRINK1/2  4-frame shrink transition → OVERWORLD
```

Player and rival names are stored in `s_player_name[8]` / `s_rival_name[8]`.
`game_get_player_name()` and `game_get_rival_name()` expose them globally.

The name grid is a 6-row × 9-col 2-D `char` array (`s_name_grid`). DEL removes
the last character; END confirms (requires at least one character). The same
grid is reused for Pokémon nickname selection (`game_nickname_open/update`).

---

## 6. World & Map System

### WorldContext (`include/world.h`)

```c
typedef struct {
    const MapHeader *map;       // current map
    const MapHeader *last_map;  // previous map (for WARP_LAST_MAP)
    PlayerState      player;
    Camera           camera;
    NpcState         npcs[16];
    u8               npc_count;
} WorldContext;

extern WorldContext g_world;
```

### MapHeader

Every map is a compile-time `const MapHeader` in `src/data/`:

```c
typedef struct {
    u8              map_id;      // MapId enum value
    const char     *name;
    const MapLayout *layout;     // grid dimensions + tileset + cell array
    const WarpEvent *warps;
    u8              warp_count;
    const NpcDef   *npcs;
    u8              npc_count;
    MapScriptFn     script;      // called once per frame from world_update()
    u16             music_id;    // AudioMusicId cast to u16
    const RoofPalette *roof_palette;
} MapHeader;
```

### MapLayout & Cells

Each cell is a `u16` (type `MapCell`) packing three fields:

| Bits | Field | Meaning |
|------|-------|---------|
| 0–9 | metatile index | which 16×16 block to draw |
| 10–11 | collision | 0 = passable, 1 = impassable |
| 12–15 | elevation | used for ledge logic |

Helper macros: `MAPCELL_METATILE(c)`, `MAPCELL_COLLISION(c)`, `MAPCELL_IMPASSABLE`.

### Tilesets & Metatiles

A `Tileset` contains raw GBA tile data (32-bit words), palette data, and an
array of `Metatile` structs. Each `Metatile` is a 2×2 block of 8×8 GBA tiles
(one for BG bottom layer, one for BG top layer), plus per-subtile palette
indices.

`tilemap_load_tileset()` uploads tiles and palettes to VRAM.
`tilemap_rebuild()` writes the full 30×20 BG tile maps from the current camera
position.
`tilemap_update_scroll()` writes `REG_BG*HOFS`/`VOFS` to scroll the background.

### Warps

```c
typedef struct {
    u8 x, y;
    u8 dest_map;    // MapId, or WARP_LAST_MAP (0xFF) = use g_world.last_map
    u8 dest_warp;   // index into destination map's warp array for spawn point
} WarpEvent;
```

`world_do_warp()` starts a black-fade-out transition. After 4 frames of
darkening (`BLDY` 0→16 in steps of 4), `world_finish_warp()` calls
`world_init()` with the destination spawn coordinates. Then the fade-in runs
(`BLDY` 16→0). The GBA brightness-blend register `0x04000050` (BLDCNT) and
`0x04000054` (BLDY) are written directly.

### Map IDs (`include/map_ids.h`)

```
0–19:   towns (Pallet, Viridian, Pewter, …)
20–39:  routes (Route 1, Route 2, …)
40+:    buildings (Oak's Lab, Player's House 1F/2F, Rival's House,
                   Viridian Mart, Viridian PokéCenter, …)
```

---

## 7. Player Movement (`src/world/player.c`)

```c
typedef struct {
    s16       tile_x, tile_y;   // logical grid position (16px units)
    s16       px, py;           // pixel position
    Direction facing;           // DOWN/UP/LEFT/RIGHT
    MoveState move_state;       // IDLE/TURNING/WALKING/FROZEN
    u8        step_frame;       // 0–15 countdown during a walk step
    s16       step_dx, step_dy; // pixel delta per frame during walk
    u8        walk_cycle;       // toggles 0/1 for walk animation
    bool8     ledge_jumping;
} PlayerState;
```

`player_update()` is called every frame from `world_update()`:
1. If `move_state == FROZEN` — skip (script control).
2. Read D-pad; if moving into a new tile, check `map_is_subtile_passable()`.
3. If facing the new direction but not yet moving → `TURNING` for 1 frame, then
   start `WALKING` (16-frame slide).
4. Each WALKING frame: advance `px`/`py` by `step_dx`/`step_dy`, decrement
   `step_frame`. At 0: snap tile coords, check warps, check NPC interaction,
   check tall grass.
5. Ledge jumping: detected by `MAPCELL_ELEVATION`. Player arcs up 8px then
   down 8px over 32 frames.

`player_script_start_step_forced()` moves the player one tile without checking
collision (used by cutscene scripts).
`player_script_is_moving()` returns TRUE while a scripted step is still
animating.

Pressing A while stationary triggers `script_trigger_npc()` for the NPC in the
tile the player is facing.

---

## 8. NPC System

### NpcDef (static, in map data)

```c
typedef struct {
    u8  x, y;           // tile position
    u8  sprite_tile;    // base OBJ tile index in NPC sprite sheet
    u8  facing;
    u8  flags;          // NPCF_HIDDEN etc.
    u16 script_id;      // dispatched by script_trigger_npc()
    u8  movement;       // NpcMovement enum
} NpcDef;
```

### NpcState (runtime, in g_world.npcs[])

Copied from `NpcDef` on `world_init()`. Fields added at runtime:
`px`, `py` (pixel position), `step_dx/dy`, `step_frame`, `walking`, `walk_cycle`,
`move_timer`.

`NPCF_HIDDEN (0x01)`: NPC is not rendered or interactive. Used to show/hide
specific NPCs based on flags (e.g. Rival disappears after battle, taken Pokéballs
disappear, Old Man swaps between sleeping and awake versions).

### NPC Movement Patterns

```c
NPC_MOVE_STAY        // stands still
NPC_MOVE_WALK_ANY    // wanders in any direction
NPC_MOVE_UP_DOWN     // only UP/DOWN
NPC_MOVE_LEFT_RIGHT  // only LEFT/RIGHT
```

`world_npcs_update()` runs every frame. Non-STAY NPCs pick a direction from
`g_vblank_count` (pseudo-random but deterministic) and check
`map_is_subtile_passable()` + player tile + NPC-vs-NPC overlap before stepping.
The `move_timer` ensures a pause between steps (`45 + i*18` frames).

---

## 9. Script System (`src/engine/script.c`)

The script system is the glue between map data, NPC interactions, cutscenes,
shops, and the Pokécenter. It is entirely driven by `script_update()` called
once per frame from `world_update()`, plus per-map `MapScriptFn` callbacks.

### Global script state

```c
static bool8  s_blocks_input     // TRUE = player cannot move or open menus
static u16    s_active_script_id // script_id of the currently running NPC
static u8     s_active_npc_index // NPC index
static u8     s_npc_script_state // 0=idle, 1=waiting for dialog, 2=cooldown
```

`script_blocks_input()` is checked by `state_overworld_update()` before
allowing the pause menu and by `player_update()` before reading D-pad.

### NPC interaction dispatch (`script_trigger_npc`)

Called from `player_update()` when A is pressed facing an NPC. The `script_id`
determines what happens:

| script_id range | Meaning |
|----------------|---------|
| 0 | unused |
| 1–9 | generic NPC texts (Oak's Lab scientist, rival's house, etc.) |
| 10–12 | Pokéball objects (Charmander/Squirtle/Bulbasaur) |
| 13–35 | map-specific NPCs (Old Man, nurse, shop clerk, etc.) |
| 21 | Viridian Old Man (pre-Pokédex path) |
| 22 | Viridian Mart clerk (post-Pokédex = opens shop) |
| 25 | Viridian PokéCenter nurse (healing flow) |
| 34 | Oak parcel handoff |

### Map script functions

Each map's `MapHeader.script` is called once per frame. Currently implemented:

- `script_pallet_town()` — the Oak-stops-player-at-Route-1-entrance cutscene
- `script_oaks_lab()` — the entire Oak's Lab sequence (walk-in, starter
  selection, rival battle, post-battle, and Pokédex delivery)
- `script_viridian_city()` — toggles sleeping/awake Old Man based on
  `FLAG_GOT_POKEDEX`
- `script_viridian_mart()` — triggers Oak's Parcel pickup on first visit

### Pallet Town cutscene state machine (`PalletScriptState`)

```
PT_IDLE              triggers at tile_y == 0 (Route 1 grass)
PT_OAK_FIRST_TEXT    "Hey! Wait! Don't go out!"
PT_OAK_WALK_TO_PLAYER Oak NPC walks to position below player
PT_OAK_SECOND_TEXT   "It's unsafe! Come with me!"
PT_OAK_LEADS_PLAYER  Oak and player walk to lab door (warp fires naturally)
```

The player's scripted path is built by `build_pallet_player_fallback_path()`,
which constructs a direction array to navigate around houses into (12,11) — the
lab door tile.

### Oak's Lab state machine (`OaksLabScriptState`, ~30 states)

Key sequence:
1. `OAKSLAB_IDLE` → detect `FLAG_FOLLOWED_OAK_INTO_LAB`, begin cutscene
2. Oak2 (hidden NPC #5) walks 3 tiles UP, swaps to Oak1 (NPC #1)
3. Player walks 8 tiles UP automatically (`OAKSLAB_PLAYER_ENTRY`)
4. Rival and Oak exchange dialog, Oak grants choose permission
   (`FLAG_OAK_ASKED_TO_CHOOSE_MON`)
5. Player presses A on a Pokéball (script_id 10/11/12 = Charmander/Squirtle/
   Bulbasaur) → Pokédex entry shown → YES/NO confirmation
6. Starter added to party, nickname offered
7. Rival walks to opposite ball using BFS pathfinding
   (`oaks_lab_find_rival_step`)
8. Player walks to battle row (y=8-9), rival approaches
9. Battle starts (`battle_setup_rival()` + `game_change_state(GAME_STATE_BATTLE)`)
10. Post-battle: rival exits, Pokédex delivery scene fires when parcel has been
    delivered (`FLAG_OAK_GOT_PARCEL`)

### Shop system (Viridian Mart)

```
SHOP_IDLE → SHOP_WELCOME → SHOP_MENU → SHOP_BUY_SELECT →
SHOP_BUY_CONFIRM → SHOP_BUY_DONE → SHOP_GOODBYE
```

`s_viridian_shop[]` defines the item list (Poké Ball $200, Antidote $100,
Parlyz Heal $200, Burn Heal $250). Purchases call `game_subtract_money()` and
`bag_add()`. The shop also draws a live money display via `shop_draw_money()`.

### PokéCenter healing flow

```
POKECENTER_IDLE → POKECENTER_WELCOME → POKECENTER_CHOICE (YES/NO) →
POKECENTER_NEED_PARTY → POKECENTER_FIT_PARTY → POKECENTER_FAREWELL →
POKECENTER_RELEASE
```

On YES: sets `g_last_healing_point` to `{MAP_VIRIDIAN_CITY, 23, 25, DIR_DOWN}`,
then calls `party_heal_all()`.

---

## 10. Battle System (`src/battle/battle_main.c`)

### Setup

```c
battle_setup_wild(species, level, player_species, nickname)
battle_setup_rival(chosen_ball, player_nickname)
```

Both fill a `BattleCtx` (static `s_battle`) and set `g_game.next_state =
GAME_STATE_BATTLE`.

### BattleCtx

```c
typedef struct {
    BattlePokemon player_mon;     // in-battle copy (stats, HP, pp, status)
    BattlePokemon enemy_mon;
    BattleState   state;
    u8            menu_cursor;    // FIGHT/BAG/POKEMON/RUN
    u8            move_cursor;
    MoveId        player_move;
    MoveId        enemy_move;
    bool8         player_goes_first;
    u16           anim_timer;
    u16           exp_gained;
    u32           player_experience;
    bool8         crit_flag;
    u16           last_damage;
    u8            last_effectiveness;
    bool8         is_wild;
    bool8         ball_caught;
    // …
} BattleCtx;
```

### Battle state machine (BattleState, ~30 states)

```
BS_INIT → BS_INTRO → BS_SEND_OUT_ENEMY → BS_SEND_OUT_PLAYER
→ BS_TURN_START → BS_PLAYER_MENU → (BS_MOVE_SELECT | BS_PARTY_MENU | BS_ITEM_MENU)
→ BS_ACTION_MSG_WAIT → BS_TURN_RESOLVE → BS_EXECUTE_MOVE → BS_EXECUTE_MOVE_WAIT
→ BS_HP_ANIM → BS_EXECUTE_EFFECT → BS_EXECUTE_EFFECT_WAIT → BS_CHECK_FAINT
→ BS_FAINT_MSG → BS_NEXT_ATTACKER → (BS_VICTORY | BS_DEFEAT)
→ BS_MONEY / BS_EXP / BS_LEVEL_UP → BS_END
```

Key rules:
- Speed determines who goes first (ties: player goes first)
- `battle_calc.c` computes damage with Gen 1 formula (no overflow protection by
  design, matching the original)
- Type effectiveness table is in `src/data/type_effectiveness.c`
- AI (`battle_ai.c`) picks moves for the enemy using a weighted random approach
- Wild battles: fleeing always succeeds; Poké Ball catch uses catch-rate formula
- Trainer battles: run ends battle (escape); no Poké Balls against trainers
- On DEFEAT: `s_blackout = TRUE`, `battle_is_blackout()` used by overworld to
  warp player to `g_last_healing_point`

---

## 11. Party System (`src/engine/party.c`, `include/party.h`)

```c
typedef struct {
    PokemonId species;
    u8        level;
    u16       dv;           // Gen 1 DVs packed into one u16
    u16       max_hp, current_hp;
    u16       attack, defense, speed, special;
    MoveId    moves[4];
    u8        pp[4];
    u8        status;       // Gen 1 status byte
    char      nickname[8];
    u32       experience;
} PartyPokemon;

typedef struct {
    u8           count;    // 0–6
    PartyPokemon mons[6];
} PartyState;

extern PartyState g_party;
```

Important functions:

| Function | Description |
|----------|-------------|
| `party_get_active()` | returns `&g_party.mons[0]` (the lead), or NULL if empty |
| `party_get_lead()` | same as above |
| `party_get_slot(n)` | returns slot n, or NULL if out of range |
| `party_set_starter(species, nickname)` | creates level-5 starter, calculates all stats |
| `party_swap_slots(a, b)` | swaps two party members |
| `party_add(mon)` | appends to party (fails if full) |
| `party_has_usable_mon(exclude)` | checks for any non-fainted mon besides the excluded slot |
| `party_heal_all()` | restores all HP and PP |
| `party_set_healing_point(map, x, y, facing)` | sets `g_last_healing_point` |

`HealingPoint` is the blackout/respawn location: map + tile coords + facing.
Defaults to `MAP_PLAYERS_HOUSE_1F` (4, 3, DIR_RIGHT).

---

## 12. Party Menu (`src/engine/party_menu.c`, `include/party_menu.h`)

Two modes:
- `PARTY_MENU_LIST` — scrollable list of party members with HP bars and icons
- `PARTY_MENU_STATUS` — detailed stats for a selected Pokémon; A swaps to lead

### GBA Palette Slot Assignments (BG palette)

| Slot | Usage |
|------|-------|
| 0 | backdrop (black) |
| 1 | text/UI font (`TEXT_PAL`) |
| 3 | HP bar |
| 8 | Pidgey party icon |
| 9 | Rattata party icon |
| 10 | Party icon (grayscale 2×2 icons, `PARTY_ICON_PAL`) |
| 11 | Bulbasaur sprite / party sprite base (`PARTY_SPRITE_PAL`) |
| 12 | Charmander sprite |
| 13 | Squirtle sprite |

**Critical rule**: palette slots must not collide. Slots 14 and 15 are
reserved for Pokédex/title graphics. If you add a new Pokémon sprite,
assign it a free slot and add it to `party_sprite_palette()` and
`prepare_party_palettes()`.

---

## 13. Item / Bag System (`src/engine/item.c`, `include/item.h`)

```c
typedef enum {
    ITEM_NONE = 0,
    ITEM_POKE_BALL,
    ITEM_POTION,
    ITEM_ANTIDOTE,
    ITEM_PARLYZ_HEAL,
    ITEM_BURN_HEAL,
    ITEM_OAKS_PARCEL,   // key item
    ITEM_TOWN_MAP,      // key item
    ITEM_COUNT,
} ItemId;
```

```c
typedef struct {
    u8 count;
    BagSlot slots[20];   // BAG_MAX_SLOTS = 20
} BagState;

extern BagState g_bag;
```

`item_is_key_item()` returns TRUE for `ITEM_OAKS_PARCEL` and `ITEM_TOWN_MAP`.
The pause menu separates regular items (ITEM menu) from key items (KEY ITEM menu).
`bag_add()` stacks identical non-key items up to 99; key items don't stack.
`bag_remove()` decrements quantity and removes the slot if it reaches 0.

---

## 14. Pokédex System (`src/engine/pokedex.c`, `include/pokedex.h`)

Two 20-byte bitmaps track seen and owned status for all 151 Pokémon. One bit
per species (index 1–151 maps to bit `(species-1) % 8` of byte `(species-1)/8`).

```c
void pokedex_set_seen(PokemonId species)
void pokedex_set_owned(PokemonId species)
bool8 pokedex_is_seen(PokemonId species)
bool8 pokedex_is_owned(PokemonId species)
```

`FLAG_GOT_POKEDEX` controls whether the POKÉDEX entry appears in the pause menu.

The list UI (`pokedex_list_open/update/close`) lets the player scroll their
Pokédex from the pause menu. The entry viewer (`pokedex_open/update/close`) is
also used during the starter-selection flow in Oak's Lab to show the Pokémon's
Pokédex entry before confirming.

---

## 15. Flags System (`src/engine/flags.c`, `include/flags.h`)

Persistent one-bit flags stored in four `u32` words (128 bits total).

```c
typedef enum {
    FLAG_GOT_STARTER = 0,
    FLAG_OAK_APPEARED_IN_PALLET,
    FLAG_FOLLOWED_OAK_INTO_LAB,
    FLAG_OAK_ASKED_TO_CHOOSE_MON,
    FLAG_BATTLED_RIVAL_IN_OAKS_LAB,
    FLAG_RIVAL_LEFT_OAKS_LAB,
    FLAG_GOT_POKEDEX,
    FLAG_OAKSLAB_CHARMANDER_TAKEN,
    FLAG_OAKSLAB_SQUIRTLE_TAKEN,
    FLAG_OAKSLAB_BULBASAUR_TAKEN,
    FLAG_STARTER_CHARMANDER,
    FLAG_STARTER_SQUIRTLE,
    FLAG_STARTER_BULBASAUR,
    FLAG_GOT_OAKS_PARCEL,
    FLAG_OAK_GOT_PARCEL,
    FLAG_COUNT,         // 15 flags as of last count
} GameFlag;
```

**Important**: never reorder or insert into the middle of this enum — existing
save files encode flags by bit position. Always append new flags before
`FLAG_COUNT`.

---

## 16. Save System (`src/engine/save.c`, `include/save.h`)

### Storage

GBA SRAM at `0x0E000000`. 8-bit bus; each byte written/read individually. The
emulator SRAM detection string `"SRAM_V113"` is embedded as a static volatile
`const char[]` so it survives optimization.

### SaveData layout (version 4, current)

```c
typedef struct {
    u8  magic[4];           // 'R','R','M','1'
    u8  version;            // 1–4
    u8  map_id;
    u8  last_map_id;
    u8  player_x, player_y;
    u8  player_facing;
    u8  player_name[8];
    u8  rival_name[8];
    u32 flags[4];           // 128 bit flag bank
    u8  option_fast_text;
    u8  option_battle_animation;
    u8  option_battle_style;
    PartyState party;
    HealingPoint last_healing_point;
    u32 money;              // added in v4
    BagState bag;           // added in v4
    u8  pokedex_seen[20];   // added in v4
    u8  pokedex_owned[20];  // added in v4
    u8  checksum;           // sum of all bytes from index 4 to checksum-1
} SaveData;
```

### Version migration

`save_read()` detects the version byte and upgrades legacy saves:
- **v1** — no party, no healing point → party cleared, default healing point
- **v2** — party without `experience` field → `experience` reconstructed from level
- **v3** — party + healing point, no money/bag/pokédex → those zeroed/cleared
- **v4** — current, read directly

`save_write()` writes payload bytes (index 4 onward) first, then the magic
bytes (0–3) last. A mid-write reset leaves the old file readable rather than
a half-written new file.

### Checksum

Single-byte sum of all bytes from index 4 to (but not including) the checksum
byte. Recomputed on write and verified on read.

---

## 17. Text & Dialog System

### Text layer (`src/engine/text.c`)

BG0 is the "text/UI" layer. `text_draw_str(col, row, str)` places characters as
tile indices into BG0's tile map. The font is a custom 8×8 pixel tile set
uploaded to VRAM on startup by `text_init()`.

Special tiles for dialog boxes (defined in `text.h`):
```
BOX_TL, BOX_TR, BOX_BL, BOX_BR, BOX_TE, BOX_BE, BOX_LE, BOX_RE, BOX_FILL
```

`text_fill_opaque()` fills every BG0 tile with the opaque background tile,
used before full-screen UI modes (trainer card, nickname screen) to prevent
the overworld from showing through.

`text_clear()` resets the entire BG0 tile map to transparent tiles.

### Dialog system (`src/engine/dialog.c`)

A scrolling two-line dialog box drawn at the bottom of the screen.

- `dialog_open()` — opens the box
- `dialog_set_text(str)` — sets the text to display (supports `\n` line breaks,
  `\f` page breaks)
- `dialog_update()` — advances letter-by-letter (or page-by-page); returns TRUE
  when the player has dismissed the final page
- `dialog_is_open()` — used by map scripts and `player_update()` to suppress input
- `dialog_yesno_open()` — overlays a YES/NO cursor on the dialog box
- `dialog_yesno_update()` — returns 1 (YES), 0 (NO), or 0xFF (pending)

`[NAME]` and `[RIVAL]` tokens in dialog strings are replaced at render time with
`game_get_player_name()` and `game_get_rival_name()`.

---

## 18. Rendering System

### Layer setup (GBA Mode 0 — 4 independent BG layers)

| Layer | Usage |
|-------|-------|
| BG0 | UI / text / dialog boxes |
| BG1 | top map tiles (trees, roofs, overlays) |
| BG2 | bottom map tiles (ground, paths, walls) |
| OBJ | sprites: player, NPCs, battle Pokémon, party icons |

### Render command queue (`src/graphics/renderer.c`, `include/render.h`)

Game code calls `render_submit(RenderCmd)` to enqueue draw operations.
`render_flush()` is called from the VBlank ISR and processes the queue,
writing directly to OAM and VRAM. This decouples game logic from hardware
timing.

```c
typedef enum {
    RCMD_NONE,
    RCMD_DRAW_SPRITE,       // regular 16×16 OBJ
    RCMD_DRAW_BG_TILE,
    RCMD_SCROLL_BG,
    RCMD_LOAD_PALETTE,
    RCMD_CLEAR_SPRITES,
    RCMD_DRAW_SPRITE_LARGE, // 32×32 OBJ (battle sprites)
} RenderCmdType;
```

`render_clear_sprites()` must be called at the start of each frame before
submitting new sprite commands (done in `world_render()` and battle rendering).

### Sprite animation

Player sprites: 6 frames stored in 3 poses × 2 (idle/walking):
- Frames 0–2: down/up/side (idle)
- Frames 3–5: down/up/side (walking, alternate step)

Walking frame selected when `move_state == WALKING` and `step_frame & 8`.
Horizontal mirroring for LEFT facing: the renderer sets the H-flip bit in OAM.

NPC sprites follow the same 3+3 frame scheme. Oak has special handling to
always use the down-facing idle frame during DOWN scripted steps (avoiding a
left-biased walking artifact).

---

## 19. Audio System (`src/audio/audio.c`, `include/audio.h`)

Custom software tracker. Audio data is in `src/audio/` with one `.c` file per
cue. `audio_update()` runs every VBlank.

### Music IDs

| ID | Cue |
|----|-----|
| `AUDIO_MUSIC_PALLET_TOWN` | Pallet Town theme |
| `AUDIO_MUSIC_TITLE_SCREEN` | Title screen |
| `AUDIO_MUSIC_TRAINER_BATTLE` | Trainer battle |
| `AUDIO_MUSIC_DEFEATED_TRAINER` | Victory fanfare |
| `AUDIO_MUSIC_ROUTES_1` | Route 1 |
| `AUDIO_MUSIC_OAKS_LAB` | Oak's Lab / intro monologue |
| `AUDIO_MUSIC_INTRO_BATTLE` | Battle intro sting |
| `AUDIO_MUSIC_MEET_PROF_OAK` | Oak encounter on Route 1 |
| `AUDIO_MUSIC_WILD_BATTLE` | Wild Pokémon battle |
| `AUDIO_MUSIC_VIRIDIAN_CITY` | Viridian City |

### SFX IDs

`AUDIO_SFX_SELECT`, `CONFIRM`, `CANCEL`, `TEXT`, `DENIED`, `START`,
`BATTLE_SELECT`, `BATTLE_CONFIRM`, `WARP_OUT`, `WARP_IN`, `PAUSE_OPEN`,
`PAUSE_CLOSE`, `CRY_BULBASAUR`, `CRY_CHARMANDER`, `CRY_SQUIRTLE`,
`CRY_WILD`.

`audio_sfx_set_battle_intro(TRUE)` enables the battle-intro stinger mode (plays
once then reverts to battle music).

---

## 20. Pause Menu

Opened by START in the overworld when player is IDLE and no dialog or script
is running. Runs as `GAME_STATE_MENU`.

Menu entries (from top):
1. **POKÉDEX** — only shown if `FLAG_GOT_POKEDEX` is set
2. **POKÉMON** — opens party menu
3. **ITEM** — opens regular-item bag
4. **KEY ITEM** — opens key-item bag
5. **[player name]** — opens Trainer Card (shows name, money, badge placeholder)
6. **SAVE** — serializes and writes `SaveData` to SRAM
7. **OPTION** — re-opens the options screen (same as title options)
8. **EXIT** — returns to overworld

Trainer Card displays player name, money (with `$` prefix), time (placeholder
"0:00"), and a badge section placeholder. The player sprite is rendered as an
OBJ at position (180, 20).

---

## 21. Data Files

| File | Contents |
|------|----------|
| `src/data/pokemon_base_stats.c` | `g_pokemon_base_stats[152]` — base HP/Atk/Def/Spd/Spc + types + catch rate + base EXP for all 151 |
| `src/data/moves.c` | Move definitions (power, type, accuracy, PP, effect) |
| `src/data/type_effectiveness.c` | Gen 1 type chart |
| `src/data/stat_stages.c` | Gen 1 stat stage multiplier table |
| `src/data/map_*.c` | Per-map layout, warp, and NPC data |
| `src/data/gfx_*.c` / `.h` | Tile + palette data for every graphical asset |
| `src/data/tileset_*.c` | Compiled tileset data |
| `src/pokemon/experience.c` | `pokemon_exp_for_level()` — Gen 1 growth rate formulas |

### Pokémon ID scheme

`PokemonId` is 1-indexed Pokédex order. Index 0 (`MON_NONE`) means "no
Pokémon". The `g_pokemon_base_stats` array has 152 entries; index 0 is padding.

---

## 22. Key Invariants & Rules

1. **Never reorder `GameFlag`** — bit positions are saved to SRAM.
2. **Never reorder `MapId`** — saved as a byte in `SaveData.map_id`.
3. **Palette slot collision is fatal** — if two systems write to the same
   palette bank the sprite colors will corrupt. Slot assignments are in
   §12 above. Always check before adding a new sprite.
4. **`text_clear()` before every full-screen mode** — and `text_fill_opaque()`
   before any mode where the overworld must be fully hidden.
5. **`render_clear_sprites()` must begin every render frame** — submitting
   sprites without clearing leaves ghost sprites from the previous frame.
6. **One-frame script cooldown** — after an NPC dialog closes,
   `s_npc_script_state = 2` holds input blocked for exactly one extra frame
   so the A press that closed the dialog cannot re-trigger the NPC.
7. **`script_reset_runtime()` on every `world_init()`** — resets all script
   state machines to IDLE so re-entering a map doesn't resume a stale cutscene.
8. **SRAM write order** — payload (bytes 4+) before magic (bytes 0–3) so
   a mid-write reset leaves the old save intact.
9. **`save_write()` returns `save_exists()`** — always verify the write by
   re-reading the magic and checksum rather than assuming success.
10. **`bag_add()` stacks regular items, not key items** — do not check
    `item_is_key_item()` before adding; `bag_add()` handles it internally.

---

## 23. Patcher (`patcher/`, `tools/build_patcher.py`)

The patcher is a single-file `patcher/index.html` generated by
`make patcher` (runs `tools/build_patcher.py`).

- Verifies the user's Pokémon Red (UE) [S][!] ROM via SHA-1
  (`ea9bcae617fdf159b045185467ae58b2e4a48b9a`)
- XOR-decodes the embedded GBA ROM using a keystream derived from
  `SHA-256(sha1_seed || chunk_index_big_endian)`, 32 bytes per chunk
- Everything runs in-browser via WebCrypto; no server or install needed
- Version is embedded from `patcher/VERSION` at build time; the file is
  bumped after each build
- Downloaded file is named `pokered_remaster_vX.Y.Z.gba`
- Deployed to GitHub Pages from the `patcher/` folder via
  `.github/workflows/pages.yml`

---

## 24. Current Progress (as of 2026-07-26)

### Implemented
- Full intro sequence (Oak's monologue, player/rival naming)
- Pallet Town, Route 1, Viridian City, all starting-area indoor maps
- Complete Oak's Lab starter selection + rival battle + Pokédex delivery
- Wild encounters on Route 1 (Pidgey, Rattata)
- Trainer battle (rival only so far)
- Battle system: turn order, damage calc, type chart, status, exp/level-up, catch
- Party menu with HP bars, Pokémon icons, status screen, LEAD swap
- Pause menu: Pokédex, party, items, key items, trainer card, save, options
- Money system with Pokemart in Viridian City
- Pokémon Center healing in Viridian City
- Full save/load with 4-version migration
- Pokédex seen/owned tracking
- Audio: all cues for implemented content
- Warp transitions (fade-out / fade-in)
- NPC movement patterns
- GitHub Pages patcher (ownership verification + XOR decode, version-stamped)

### In Progress / Known Issues
- Pokédex delivery scene accuracy (`script_oaks_lab` Pokédex states)
- No gym battles or story content beyond Viridian City
- Trainer card shows "0:00" (no play-time tracking)
- Badge section on trainer card is a placeholder
