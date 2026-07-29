# Pokemon Red Remaster

Pokemon Red Remaster is an in-progress Game Boy Advance reimplementation of
the opening Pokemon Red experience. It is written in C and targets original
GBA hardware through devkitPro, devkitARM, and libgba.

This is an experimental fan project. Pokemon, character names, original game
content, and reference assets belong to their respective copyright holders.
Do not distribute commercial game ROMs or other copyrighted material with
this source tree.

## Current playable slice

The current build is a playable early-game slice covering Pallet Town, Route 1,
Route 2, Route 22, Viridian City, Viridian Forest, Pewter City, their currently
ported gates, and the implemented reference-linked interiors. It includes:

- The title, intro, naming, Oak's Lab starter selection, Pokédex delivery, and
  scripted rival sequence.
- Data-driven maps with compatible tilesets, collision, ledges, camera
  scrolling, step-on and boundary warps, map connections, signs, item balls,
  NPC dialogue, persistent flags, and fade transitions.
- All 151 species' gameplay data, sprites, palettes, Pokédex entries, initial
  moves, and level-up learnsets. The patcher reports the current species data
  as 151/151.
- Party and PC storage, party selection, party switching, nicknames, stats,
  experience, status conditions, move lists, Pokédex seen/owned tracking, and
  consistent species palettes across battle, party, and Pokédex screens.
- Wild and trainer battles with data-driven encounter tables and trainer
  parties, trainer line-of-sight approach/dialogue, move progression, status
  effects, catching, Poké Ball animation, battle item pages, switching,
  experience, level-up moves, post-battle evolution, and blackout recovery.
- A data-driven item system with shops, map item balls, evolution stones,
  party-member selection, TM/HM compatibility and teaching, move replacement,
  regular-item scrolling, key items, and money handling. Trainer battles use
  pokered-style blackout money loss and display the amount lost.
- SRAM save/load with checksum validation, save-version migration, money,
  bag contents, party, PC boxes, Pokédex state, healing points, map position,
  flags, hidden objects, and completed scripts.
- The single-file ownership-verification patcher, currently generated as
  v0.0.58 in `patcher/index.html`, with progress counts for maps, Pokémon,
  moves, music, items, sprites, learnsets, and Pokédex entries.

## Current limitations and next work

- The project is not yet a complete Red world port. Gym interiors and battles,
  most towns/routes/buildings, the full story script, and many reference NPCs,
  cutscenes, dialogues, encounters, and item events still need conversion.
- The Trainer Card time and badge sections are placeholders.
- The current implementation has no automated unit-test suite or emulator-
  driven integration test; verification is currently build-based and manual in
  an emulator.
- Trade evolution requirements are intentionally replaced with level-up paths;
  link trading is outside the project scope.

## Repository layout

    .
    |-- Makefile                         GBA build rules
    |-- build.sh                         Git Bash/MSYS convenience wrapper
    |-- link.ld                          Reference linker script
    |-- assets/                          Project-owned source artwork/assets
    |-- include/                         Public headers and shared data types
    |-- src/
    |   |-- battle/                      Battle state machine and battle rules
    |   |-- data/                        Maps, Pokemon, moves, tiles, graphics
    |   |-- engine/                      Game state, scripts, dialog, text, flags
    |   |-- graphics/                    Hardware renderer and sprite submission
    |   |-- input/                       GBA key input handling
    |   `-- world/                       Maps, player movement, tilemaps, NPCs
    |-- tools/                           Asset conversion scripts
    |-- refs/                            Local pokered checkout; ignored
    |-- build/                           Object/dependency/map output; ignored
    `-- README.md

The local `refs/pokered/` checkout is deliberately ignored and is intended for
developer reference only. Project documentation such as
`refs/GAME_ARCHITECTURE.md` is tracked; generated assets needed by the GBA
build are tracked in this repository.

## Development environment

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

The Makefile uses DEVKITPRO and DEVKITARM. If unset, it defaults to the
common MSYS2 paths /c/devkitpro and /c/devkitpro/devkitARM.

Git Bash example:

    export DEVKITPRO=/c/devkitpro
    export DEVKITARM=/c/devkitpro/devkitARM
    export PATH="$DEVKITARM/bin:$DEVKITPRO/tools/bin:$PATH"

PowerShell example:

    $env:DEVKITPRO = 'C:/devkitpro'
    $env:DEVKITARM = 'C:/devkitpro/devkitARM'
    $env:Path = "$env:DEVKITARM/bin;$env:DEVKITPRO/tools/bin;$env:Path"

The normal C build does not require Node.js, Python, CMake, or a separate
package manager.

## Building the ROM

Standard build:

    make

The output is `pokered_remaster.gba`. The build discovers C and assembly
sources, compiles them for the ARM7TDMI in Thumb mode, links against libgba,
converts the ELF to a raw GBA binary, and runs gbafix.

Use parallel compilation when appropriate:

    make -j2

Clean rebuild:

    make clean
    make -j2

`make clean` removes local build output, the ELF, and the GBA output. It does
not remove source files, reference assets, saves, or session data.

The convenience wrapper assumes the checkout is mounted as `/e/pokemon
recomp` and may need editing elsewhere:

    bash build.sh
    bash build.sh -j2
    bash build.sh clean

Typical local build output includes:

    pokered_remaster.gba
    pokered_remaster.elf
    build/**/*.o
    build/**/*.d
    build/pokered_remaster.map

These are local build artifacts and are ignored by Git.

## Program architecture

`src/engine/game.c` owns the top-level states:

- `GAME_STATE_BOOT`
- `GAME_STATE_TITLE`
- `GAME_STATE_INTRO`
- `GAME_STATE_OVERWORLD`
- `GAME_STATE_BATTLE`
- `GAME_STATE_MENU`

`src/engine/script.c` owns scripted events and flags. The Oak's Lab flow
selects and removes the starter Pokeball, handles nickname entry, moves the
rival, selects the rival's starter, and starts the battle.

Battle modules are split by responsibility:

- `src/battle/battle_main.c`: battle context, state machine, HUD, menus,
  messages, animations, and turn sequencing.
- `src/battle/battle_pokemon.c`: runtime Pokemon initialization, HP/stat
  calculation, DVs, and stat-stage scaling.
- `src/battle/battle_calc.c`: accuracy, critical hits, damage, and helpers.
- `src/battle/battle_ai.c`: rival move selection.
- `src/battle/battle_rng.c`: battle random number generation.
- `src/pokemon/experience.c`: species growth curves and level thresholds.

Party and status UI live in `src/engine/party.c` and
`src/engine/party_menu.c`. Nicknames are stored in the party record and are
used everywhere a Pokemon name is displayed, with the species name as the
fallback when no nickname exists.

World behavior is split between `src/world/player.c`, `map.c`, `world.c`, and
`tilemap.c`. Exact warp coordinates are walkable and trigger after the player
finishes stepping onto them; boundary warps still resolve when leaving the
map. Map flags and script state are restored when a save is loaded.

## Graphics and palettes

Graphics are embedded in tracked C and assembly data under `src/data/`.
Tileset definitions assign collision behavior, metatiles, and palette data.
The indoor palette system is defined in:

- `include/indoor_palette.h`
- `src/data/indoor_palette.c`
- `src/data/tileset_house.c`
- `src/data/tileset_house_general.c`
- `src/data/tileset_gym.c`

Indoor tile palettes use per-tile maps and are based on the local
pokemon-rgb reference. Tilemap loading also preserves indoor tile 0 during
room transitions so wall graphics do not become blank or produce transition
artifacts.

Battle sprites use 64x64 GBA sprite canvases for Pokemon. Red's dedicated
battle back picture is a separate 32x32 source asset and is centered inside
that canvas. Trainer intro sprites use their own source dimensions and tile
stride rather than being treated as Pokemon sprites.

## Reference and asset workflow

The project uses pret's pokered repository as a local reference. Clone it
after cloning this project:

    git clone https://github.com/pret/pokered.git refs/pokered

The reference checkout is ignored and must not be committed. Update it with:

    git -C refs/pokered fetch origin
    git -C refs/pokered pull --ff-only

For reproducible asset work, record the reference revision:

    git -C refs/pokered rev-parse HEAD

Optional conversion scripts require Windows PowerShell 5+ or PowerShell 7+,
ffmpeg on PATH, and the expected pokered PNG files. Generated C arrays are
checked in and asset conversion is not part of the normal build.

Battle sprites:

    .\tools\convert_battle_sprites.ps1

NPC sprites:

    .\tools\convert_npc_sprites.ps1

The battle converter expects pokered front and back sprites for Bulbasaur,
Charmander, and Squirtle. After regeneration, perform a clean build and
inspect the graphics in an emulator.

## Controls

- D-pad: move the player, navigate menus, and navigate the nickname editor.
- A: interact, confirm, select a move, and advance prompts.
- B: cancel/back out of move, party, and item screens.
- Start: open the pause menu from the overworld.
- Select: reserved or context-dependent in the current slice.

## Save data and emulator setup

The ROM declares the standard `SRAM_V113` backup marker and uses the GBA's
32 KiB byte-addressable SRAM region. The pause-menu SAVE entry verifies the
SRAM write-back and checksum before showing success.

For mGBA, VBA-M, and compatible emulators, use automatic detection or force
`SRAM` / `SRAM 32K`. Do not use EEPROM or Flash. If an emulator caches backup
settings for a ROM filename, use a new filename or clear its cartridge
configuration.

Verify the marker after building:

    rg -a -o "SRAM_V113" pokered_remaster.gba

When testing script persistence, save after completing a scripted event and
reload the save. Starter Pokeballs, hidden NPCs, completed flags, party
stats, nicknames, map position, and script progression should remain intact.

## Testing and development workflow

Useful checks before committing:

    git status --short --branch
    git diff --check
    make clean
    make -j2

Test in an emulator from a fresh save and exercise title, introduction,
movement, house and lab warps, starter selection, nickname entry, rival
approach, battle introduction, move execution, wild encounters, Run, party
list/status screens, save/load, and battle exit.

For bug reports, include the emulator/version, fresh or continued save,
selected starter/nickname, exact input sequence, last visible message, a
screenshot when graphical, and the commit or ROM build used.

Known non-fatal compiler warnings may include unused helpers, title arithmetic
parentheses, and discarded volatile qualifiers in tile overlay helpers. New
warnings should be investigated.

## Contributing and publishing

Review the staged list explicitly before committing:

    git add include src tools assets Makefile build.sh link.ld README.md
    git diff --cached --stat
    git diff --cached --name-only

Do not use `git add -A` in a workspace containing local ROMs, saves, reference
checkouts, debugger output, or conversation exports unless the staged list has
been reviewed. Never commit machine-specific absolute paths, usernames,
temporary files, ROMs, saves, emulator states, or session exports.

Suggested commit prefixes are `battle:`, `world:`, `graphics:`, `script:`,
`build:`, and `docs:`.

## License and attribution

This repository is an educational and experimental fan project. Additions
must respect the licenses and attribution requirements of external code,
graphics, tools, and reference data. Keep external reference checkouts
outside the published source tree unless their license explicitly permits
redistribution.
