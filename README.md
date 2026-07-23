# Pokémon Red Remaster

Pokémon Red Remaster is an in-progress Game Boy Advance reimplementation of the opening Pokémon Red experience. It is written in C and targets original GBA hardware through devkitPro, devkitARM, and libgba.

The current playable slice includes the title and introduction flow, opening overworld areas, Oak's Lab starter selection, starter nickname entry, Oak's Lab rival movement, and a first-pass turn-based rival battle. Battle presentation and data are developed against a local pokered reference checkout.

This is an experimental fan project. Pokémon, character names, original game content, and reference assets belong to their respective copyright holders. Do not distribute commercial game ROMs or other copyrighted material with this source tree.

## Current status

Implemented or partially implemented:

- GBA boot, title, introduction, and overworld state flow.
- Pallet Town, Oak's Lab, Route 1, Red's House 1F, and Red's House 2F.
- Player movement, NPC movement, map transitions, collision, and scripted Oak's Lab events.
- Starter selection for Bulbasaur, Charmander, and Squirtle.
- Starter nickname entry and nickname display in battle UI and messages.
- Rival starter selection using the opposite-starter mapping from Pokémon Red.
- Battle HUD with Pokémon sprites, names, levels, HP values, and HP bars.
- Pokered-derived front and back battle sprite conversion for the three starter species.
- Fight, PKMN, Item, and Run battle menu entries.
- Two starter moves per species, move selection, PP, damage, accuracy, critical hits, type effectiveness, stat stages, fainting, experience, and a level-up path.

Known limitations:

- The party currently contains only the active starter. The PKMN screen is a functional placeholder and reports that there are no other Pokémon to use.
- There is no inventory or item database yet. The Item screen currently reports "No items!" and returns to the battle menu.
- Run is rejected during the Oak's Lab rival trainer battle, matching trainer-battle rules. Wild battles and escape calculations are not implemented.
- Only the opening species, moves, maps, and scripted battle are represented.
- There is no complete save/load system. Local .sav files are ignored.
- There is no automated unit-test suite or emulator-driven integration test.

## Repository layout

    .
    ├── Makefile                         GBA build rules
    ├── build.sh                         Git Bash/MSYS convenience wrapper
    ├── link.ld                          Reference linker script
    ├── assets/                          Project-owned source artwork/assets
    ├── include/                         Public headers and shared data types
    ├── src/
    │   ├── battle/                      Battle state machine and battle rules
    │   ├── data/                        Maps, Pokémon, moves, tiles, graphics
    │   ├── engine/                      Game state, scripts, dialog, text, flags
    │   ├── graphics/                    Hardware renderer and sprite submission
    │   ├── input/                       GBA key input handling
    │   └── world/                       Maps, player movement, tilemaps, NPCs
    ├── tools/                           Asset conversion scripts
    ├── refs/                            Local pokered checkout; intentionally ignored
    ├── build/                           Object/dependency/map output; ignored
    ├── debug/                           Local debugger output; ignored
    └── tmp/                             Local temporary files; ignored

The refs/ directory is deliberately not published. It is intended for a developer's local checkout of pokered. Generated assets needed by the GBA build, such as src/data/gfx_battle_sprites.c, are tracked in this repository.

## Required development environment

### Toolchain

Install:

1. devkitPro package manager.
2. devkitARM, including arm-none-eabi-gcc and binutils.
3. libgba, including GBA headers and libraries.
4. devkitPro tools, including gbafix.
5. GNU Make.
6. A POSIX-compatible shell when using build.sh, such as Git Bash or MSYS2.

The following commands should be available:

    arm-none-eabi-gcc
    arm-none-eabi-objcopy
    gbafix
    make

The Makefile uses DEVKITPRO and DEVKITARM. If unset, it defaults to the common MSYS2 paths /c/devkitpro and /c/devkitpro/devkitARM. On another installation, set them before building.

Git Bash example:

    export DEVKITPRO=/c/devkitpro
    export DEVKITARM=/c/devkitpro/devkitARM
    export PATH="$DEVKITARM/bin:$DEVKITPRO/tools/bin:$PATH"

PowerShell example:

    $env:DEVKITPRO = 'C:/devkitpro'
    $env:DEVKITARM = 'C:/devkitpro/devkitARM'
    $env:Path = "$env:DEVKITARM/bin;$env:DEVKITPRO/tools/bin;$env:Path"

The normal build does not require Node.js, Python, CMake, or a separate package manager.

### Optional asset-generation tools

The battle sprite conversion script additionally requires Windows PowerShell 5+ or PowerShell 7+, ffmpeg on PATH, and a local pokered reference checkout containing the expected PNG files.

Asset conversion is not part of the normal C build because the generated C array is checked in.

## Getting the source

    git clone https://github.com/johnson-cooper/POKERED-REMASTERED.git
    cd POKERED-REMASTERED
    git status --short --branch

The repository intentionally excludes:

- .gb and .gba ROM files.
- .sav and emulator state files.
- Object files, ELF files, map files, and build directories.
- .claude/, .agents/, session handoffs, conversation exports, and local debugging artifacts.
- The local refs/ pokered checkout.

## Building the ROM

### Standard build

From Git Bash, MSYS2, or a shell where GNU Make and devkitARM are available:

    make

The default output is pokered_remaster.gba.

The build:

1. Discovers C and assembly sources under the directories listed in SRCDIRS.
2. Compiles C for the ARM7TDMI processor in Thumb mode.
3. Compiles assembly assets with the C preprocessor enabled.
4. Links against libgba.
5. Writes pokered_remaster.elf and build/pokered_remaster.map.
6. Converts the ELF to a raw GBA binary with arm-none-eabi-objcopy.
7. Runs gbafix to write the title, game code, and header checksum.

Use parallel compilation:

    make -j2

### Clean rebuild

    make clean
    make -j2

make clean removes the local build/ directory, ELF, and GBA output. It does not remove source files, reference assets, save files, or session data.

### Convenience wrapper

build.sh sets the common devkitPro paths and calls Make. It assumes the original checkout is mounted as /e/pokemon recomp, so it may need editing after cloning elsewhere:

    bash build.sh
    bash build.sh -j2
    bash build.sh clean

For a different checkout, prefer make directly or update the cd line in build.sh.

## Build output and generated files

Typical local output includes:

    pokered_remaster.gba
    pokered_remaster.elf
    build/**/*.o
    build/**/*.d
    build/pokered_remaster.map

These are local build artifacts and are ignored by Git. The source of truth is the C, assembly, header, map, graphics, and tool files tracked in the repository.

## Development workflow

Use this loop:

    git status --short --branch
    # edit source files
    git diff --check
    make -j2
    git status --short --branch

Before committing, review the staged list explicitly:

    git add include src tools assets Makefile build.sh link.ld README.md
    git diff --cached --stat
    git diff --cached --name-only

Do not use git add -A in a workspace containing local ROMs, saves, references, or session data unless the staged file list has been reviewed.

Recommended commit prefixes:

- battle: battle rules, battle UI, or battle state changes.
- world: maps, movement, collision, or overworld changes.
- graphics: tiles, palettes, sprites, or renderer changes.
- script: event and cutscene changes.
- build: Makefile, linker, toolchain, or asset pipeline changes.
- docs: README and developer documentation changes.

Never put machine-specific absolute paths, usernames, temporary directories, ROMs, saves, or emulator state into tracked files.

## Program architecture

### Top-level game state

src/engine/game.c owns the top-level state machine:

- GAME_STATE_BOOT: hardware and initial runtime setup.
- GAME_STATE_TITLE: title screen and title input.
- GAME_STATE_INTRO: opening introduction sequence.
- GAME_STATE_OVERWORLD: map, player, NPC, and script updates.
- GAME_STATE_BATTLE: battle initialization and frame updates.
- GAME_STATE_MENU: reserved for broader menu work.

Use game_change_state() for transitions. Battle entry calls battle_init(). Battle exit restores the overworld display state and preserves Oak's Lab context.

### Scripts and events

src/engine/script.c contains the current event state machines. The Oak's Lab flow:

1. Selects one of three Poké Balls.
2. Sets the starter flag and hides the selected ball/NPC.
3. Asks whether to nickname the starter.
4. Runs the nickname editor when requested.
5. Moves the rival through Oak's Lab.
6. Calls battle_setup_rival() and enters GAME_STATE_BATTLE.

Keep event decisions in the script layer. Do not launch battles directly from rendering or input code.

### Battle system

- src/battle/battle_main.c: battle context, state machine, HUD, menus, messages, animations, and turn sequencing.
- src/battle/battle_pokemon.c: runtime Pokémon initialization, HP/stat calculation, DVs, and stat-stage scaling.
- src/battle/battle_calc.c: accuracy, critical hit, damage, and physical/special helpers.
- src/battle/battle_ai.c: rival move selection.
- src/battle/battle_rng.c: battle random number generation.
- src/data/moves.c: move definitions and move names.
- src/data/pokemon_base_stats.c: base stats and species data.
- src/data/type_effectiveness.c: type matchup table.

The battle flow is broadly:

    initialize → intro → enemy send-out → player send-out → action menu
    → fight/move selection or another action screen → turn resolution
    → move execution and damage animation → faint/experience/level checks
    → victory or defeat → return to overworld

The action menu follows the pokered layout:

    FIGHT  PKMN
    ITEM   RUN

Fight enters move selection. PKMN displays the single active starter and reports that no other Pokémon are available. Item displays an empty item state. Run is disallowed for the rival trainer battle.

### Pokémon nicknames

The nickname editor lives in src/engine/game.c. Oak's Lab copies the result into the script runtime, then passes it to battle_setup_rival(). The battle context stores the pointer for the battle lifetime and uses the nickname in HUDs and battle messages, falling back to the species name when no nickname was entered.

If the Pokémon storage model changes, update nickname ownership and lifetime rules. Battle text must never point at a temporary stack buffer.

### Rendering and graphics

- src/graphics/renderer.c: render command submission and sprite/OAM work.
- src/engine/text.c: text tiles, boxes, palettes, and text drawing.
- src/data/gfx_*.c and src/data/gfx_*.s: embedded graphics data.
- src/data/tileset_*.c: map tile and metatile definitions.
- src/data/gfx_battle_sprites.c: generated 4bpp battle sprite arrays.

Battle sprites are 64×64 GBA sprite canvases. The player uses the back sprite and the opponent uses the front sprite. The conversion maps pokered grayscale shades to species object-palette indices.

## Pokered reference workflow

The project uses pret's pokered repository as its local reference. Clone it
from GitHub after cloning this project:

    git clone https://github.com/pret/pokered.git refs/pokered

This command must be run from the project root. The resulting layout should
be:

    POKERED-REMASTERED/
    ├── refs/
    │   └── pokered/
    ├── src/
    └── README.md

The reference checkout is intentionally ignored by this repository, so it
will not be included in commits or pushes. To update an existing checkout:

    git -C refs/pokered fetch origin
    git -C refs/pokered pull --ff-only

For reproducible asset work, record the pokered commit used when regenerating
graphics:

    git -C refs/pokered rev-parse HEAD

Place the checkout at:

    refs/pokered/

The converter expects:

    refs/pokered/gfx/pokemon/front/bulbasaur.png
    refs/pokered/gfx/pokemon/front/charmander.png
    refs/pokered/gfx/pokemon/front/squirtle.png
    refs/pokered/gfx/pokemon/back/bulbasaurb.png
    refs/pokered/gfx/pokemon/back/charmanderb.png
    refs/pokered/gfx/pokemon/back/squirtleb.png

Run it from PowerShell:

    .\tools\convert_battle_sprites.ps1

For another checkout location:

    .\tools\convert_battle_sprites.ps1 -ReferenceDir 'E:\path\to\refs\pokered\gfx\pokemon' -OutputFile 'E:\path\to\POKERED-REMASTERED\src\data\gfx_battle_sprites.c'

After regeneration, perform a clean build and inspect all six sprites in an emulator. Do not regenerate from an unknown reference revision without recording the change.

## Controls

- D-pad: move the player, navigate menus, and navigate the nickname editor.
- A: interact, confirm, select a move, and advance prompts.
- B: cancel/back out of move, PKMN, and Item screens.
- Start/Select: reserved or context-dependent in the current slice.

The key constants are defined in include/gba.h and consumed through include/input.h. Use input_pressed() for one-frame actions and input_held() only when a held button is intentional.

## Running and testing the ROM

1. Build pokered_remaster.gba.
2. Open it in a GBA emulator such as mGBA, or transfer it to compatible development hardware.
3. Start a new emulator save when testing the opening sequence.
4. Exercise title, introduction, movement, starter selection, nickname flow, rival approach, battle menu, move execution, and battle exit.

For bug reports, include emulator/version, new or continued save state, selected starter/nickname, exact input sequence, last visible message, screenshot if graphical, and the commit/ROM build used.

Validation checklist:

    git diff --check
    make clean
    make -j2

Then verify:

- the ROM was created and gbafix completed;
- no compiler errors occurred;
- warnings are understood and not introduced by the change;
- the expected state or screen is reachable;
- text fits inside intended boxes;
- sprites have the correct orientation and palette;
- HP bars remain inside HUD boxes;
- nickname and species fallback text both work;
- git status contains no unintended generated or local files.

The current build emits known non-fatal warnings for unused helpers, title arithmetic parentheses, and intentional map array override initializers. New warnings should be investigated.

## Troubleshooting

### Compiler not found

Install devkitARM and add its bin directory to PATH:

    which arm-none-eabi-gcc
    arm-none-eabi-gcc --version

### devkitARM paths cannot be resolved

Set DEVKITPRO and DEVKITARM to the actual installation paths, then run make clean and make.

### libgba or gba.specs cannot be found

Verify:

    $DEVKITARM/arm-none-eabi/lib/gba.specs
    $DEVKITPRO/libgba/include
    $DEVKITPRO/libgba/lib

### gbafix cannot be found

Add $DEVKITPRO/tools/bin to PATH or set DEVKITPRO correctly.

### Sprite conversion fails

Confirm PowerShell, ffmpeg, and all six pokered PNG paths. Pass -ReferenceDir and -OutputFile for a non-default checkout.

### Graphics look wrong but build succeeds

Check palette indices, tile dimensions, OAM coordinates, and text tilemap positions. A compiler cannot detect sprites outside the screen, overlapping HUD borders, or the wrong object palette.

### A battle starts repeatedly after returning to the overworld

Check the battle-complete flag and previous-state handling in src/engine/game.c and src/engine/script.c. Battle exit must return to the existing Oak's Lab context and not rerun the completed script branch.

## Extending the project

### Adding a Pokémon or move

1. Add or extend the species/move identifier.
2. Add base stats or move data.
3. Add front/back graphics and palette mapping if battle-ready.
4. Update battle sprite selection and species-name fallback logic.
5. Update starter/rival setup only when appropriate.
6. Build cleanly and test HUD, menus, moves, and battle outcomes.

Explicit tables are used in several places, so adding an enum value does not automatically add graphics, text, stats, moves, or a valid party.

### Adding a map or event

A map change may require updates to:

- include/map_ids.h;
- src/data/map_*.c;
- src/data/tileset_*.c;
- NPC/object definitions;
- collision or warp handling in src/world/;
- event logic in src/engine/script.c.

Test entry and exit warps from both directions. Test scripted events from a fresh state because flags and hidden NPC state can mask bugs on repeated runs.

## Publishing changes

Before publishing:

    git status --short --branch
    git diff --check
    git diff --stat
    make clean
    make -j2

Review the staged list and confirm it contains no ROMs, saves, reference checkout, debugger output, .claude/, .agents/, or conversation exports. Publish source and project-owned assets, not local development state.

## License and attribution

This repository is an educational and experimental fan project. Additions should respect the licenses and attribution requirements of external code, graphics, tools, and reference data. Keep external reference checkouts outside the published source tree unless their license explicitly permits redistribution.
