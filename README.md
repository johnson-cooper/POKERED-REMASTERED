# Pokemon Red Remaster

Pokemon Red Remaster is an in-progress Game Boy Advance reimplementation of
the opening Pokemon Red experience. It is written in C and targets original
GBA hardware through devkitPro, devkitARM, and libgba.

This is an experimental fan project. Pokemon, character names, original game
content, and reference assets belong to their respective copyright holders.
Do not distribute commercial game ROMs or other copyrighted material with
this source tree.

## Current playable slice

The current build includes:

- GBA boot, title screen, introduction flow, audio cues, and game-state flow.
- Pallet Town, Oak's Lab, Route 1, Red's House 1F/2F, and Rival's House.
- Player movement, walking animation, NPC movement, collision, camera
  scrolling, map-edge warps, walkable step-on warps, collision warps, and
  fade transitions.
- Scripted Oak's Lab events: starter selection, Oak's dialogue, starter
  nickname entry, persistent Pokeball removal, rival movement, rival starter
  selection, and the rival battle.
- Bulbasaur, Charmander, and Squirtle starter selection with the correct
  opposite-starter rival mapping.
- Persistent party data for species, nickname, level, DVs, stats, HP, status,
  moves, PP, and experience.
- Full-screen party list and status screens with animated party icons,
  nickname/species fallback, level, HP, status, stats, types, total experience,
  and experience remaining to the next level.
- Species-specific experience growth curves and level-up stat/HP updates.
- Wild and trainer battles with battle introductions, trainer and Pokemon
  sprites, front/back sprites, send-out sequencing, cries, move selection,
  PP, accuracy, critical hits, type effectiveness, stat stages, fainting,
  experience, leveling, victory/defeat handling, and wild-battle escape
  attempts.
- Battle HUDs with Pokemon names or nicknames, levels, HP values, and HP bars.
- Current starter moves and rival battle AI.
- Pokedex species screens for the implemented starter entries, including
  white backgrounds, sprite transparency, and species palettes.
- Colorized overworld graphics, flowers, NPCs, player sprite, signs, fences,
  houses, and Oak's House palette corrections.
- Colorized indoor maps using per-tileset palettes for Red's House, Rival's
  House, and Oak's Lab, with tile-specific palette assignments and
  transition-safe tile loading.
- SRAM save/load with checksum validation, player/map position, party data,
  healing point, options, flags, hidden objects, completed scripts, and Oak's
  Lab progression state.

## Current limitations

- The party model currently starts with one active starter. Additional party
  acquisition, party switching, storage, and party-wide battle logic are not
  implemented yet.
- There is no inventory or item database. The Item screen reports "No items!".
- Run is unavailable in trainer battles and available only in wild battles.
- The Pokedex currently exposes the implemented starter species rather than a
  complete Pokemon database flow.
- Only the opening maps, species, moves, NPCs, and scripted sequence are
  represented.
- There is no automated unit-test suite or emulator-driven integration test.

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

The refs/ directory is deliberately not published. It is intended for a
developer's local checkout of pokered. Generated assets needed by the GBA
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
