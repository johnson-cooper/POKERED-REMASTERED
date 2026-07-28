"""Generate the first pokered Forest tileset/map data slice."""

from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
REF = ROOT / "refs" / "pokered"


def c_words(image_path: Path) -> list[int]:
    image = Image.open(image_path).convert("L")
    if image.size != (128, 48):
        raise ValueError(f"unexpected tileset size: {image.size}")
    words: list[int] = []
    for tile_y in range(0, 48, 8):
        for tile_x in range(0, 128, 8):
            rows = []
            for y in range(tile_y, tile_y + 8):
                value = 0
                for x in range(tile_x, tile_x + 8):
                    shade = image.getpixel((x, y))
                    # pokered's four-shade atlas is black-to-white. GBA
                    # 4bpp uses the same four visible indices as the existing
                    # converted overworld assets: 0, 5, A, F.
                    nibble = min(3, (shade * 4) // 256) * 5
                    value |= nibble << (x - tile_x) * 4
                rows.append(value)
            words.extend(rows)
    return words


def emit_words(path: Path, name: str, words: list[int]) -> None:
    lines = [
        '#include "gba.h"',
        '#include "gfx_forest.h"',
        "",
        f"const u32 {name}[{len(words)}] = {{",
    ]
    for i in range(0, len(words), 8):
        lines.append("    " + ", ".join(f"0x{word:08X}" for word in words[i : i + 8]) + ",")
    lines += ["};", f"const u32 {name[:-6]}tile_count = 96;", ""]
    path.write_text("\n".join(lines), encoding="ascii")


def emit_tileset(path: Path, blocks: bytes) -> None:
    lines = [
        '#include "world.h"',
        '#include "gfx_forest.h"',
        '#include "gfx_overworld.h"',
        '#include "overworld_palette_map.h"',
        "",
        "#define ZERO16 {0}",
        "",
        f"static const Metatile s_forest_metatiles[{len(blocks) // 16}] = {{",
    ]
    for block in range(len(blocks) // 16):
        tiles = ", ".join(f"0x{v:02X}" for v in blocks[block * 16 : block * 16 + 16])
        lines.append(f"    [{block}] = {{ .bottom={{ {tiles} }}, .top=ZERO16, .palettes=ZERO16, .top_palettes=ZERO16 }},")
    lines += [
        "};",
        "",
        "static const u8 s_forest_collision[] = {",
        "    0x1E,0x20,0x2E,0x30,0x34,0x37,0x39,0x3A,",
        "    0x40,0x51,0x52,0x5A,0x5C,0x5E,0x5F,",
        "};",
        "",
        "const Tileset g_tileset_forest = {",
        "    .tiles = g_forest_tiles,",
        "    .overlay_tiles = g_forest_tiles,",
        "    .tile_count = 96,",
        "    .palettes = g_overworld_palette_colors,",
        "    .palette_count = 13,",
        "    .palette_profile = &g_overworld_palette_profile,",
        "    .tile_palette_map = g_overworld_tile_palette_map,",
        "    .metatiles = s_forest_metatiles,",
        f"    .metatile_count = {len(blocks) // 16},",
        "    .use_cell_collision = TRUE,",
        "    .collision_tiles = s_forest_collision,",
        "    .collision_tile_count = ARRAY_COUNT(s_forest_collision),",
        "};",
        "",
        "#undef ZERO16",
    ]
    path.write_text("\n".join(lines), encoding="ascii")


def emit_map(path: Path, name: str, block_name: str, width: int, height: int, blocks: bytes, warps: str, body: str, tileset: str) -> None:
    map_suffix = name[len("g_map_") :]
    cells = ", ".join(f"P(0x{v:02X})" for v in blocks)
    lines = [
        '#include "world.h"',
        '#include "audio.h"',
        '#include "map_ids.h"',
        '#include "gfx_npcs.h"',
        '#include "gfx_npcs_extra.h"',
        '#include "gfx_pokeball.h"',
        '#include "item.h"',
        '#include "flags.h"',
        '#include "trainer_parties.h"',
        "",
        f"extern const Tileset {tileset};",
        "#define P(m) MAPCELL_MAKE((m), 0, 0)",
        "",
        f"static const MapCell s_{block_name}_cells[{width} * {height}] = {{",
    ]
    for i in range(0, len(blocks), 12):
        lines.append("    " + ", ".join(cells.split(", ")[i : i + 12]) + ",")
    lines += [
        "};",
        "",
        f"static const MapLayout s_{block_name}_layout = {{",
        f"    .width = {width}, .height = {height}, .tileset = &{tileset},",
        f"    .cells = s_{block_name}_cells,",
        "};",
        "",
        warps,
        body,
        f"const MapHeader {name} = {{",
        f"    .map_id = MAP_{map_suffix.upper()},",
        f"    .name = \"{map_suffix.replace('_', ' ')}\",",
        f"    .layout = &s_{block_name}_layout,",
        f"    .warps = s_{block_name}_warps, .warp_count = ARRAY_COUNT(s_{block_name}_warps),",
        "    .npcs = s_npcs, .npc_count = ARRAY_COUNT(s_npcs),",
        "    .script = NULL, .music_id = AUDIO_MUSIC_ROUTES_1, .roof_palette = NULL,",
        "    .bg_events = s_bg_events, .bg_event_count = ARRAY_COUNT(s_bg_events),",
        "};",
        "#undef P",
        "",
    ]
    path.write_text("\n".join(lines), encoding="ascii")


def main() -> None:
    data = ROOT / "src" / "data"
    emit_words(data / "gfx_forest.c", "g_forest_tiles", c_words(REF / "gfx/tilesets/forest.png"))
    (data / "gfx_forest.h").write_text(
        '#pragma once\n#include "types.h"\nextern const u32 g_forest_tiles[];\nextern const u32 g_forest_tile_count;\n',
        encoding="ascii",
    )
    forest_blocks = (REF / "gfx/blocksets/forest.bst").read_bytes()
    emit_tileset(data / "tileset_forest.c", forest_blocks)
    forest_map = (REF / "maps/ViridianForest.blk").read_bytes()
    warps = """static const WarpEvent s_viridian_forest_warps[] = {
    { .x=1, .y=0, .dest_map=MAP_VIRIDIAN_FOREST_NORTH_GATE, .dest_warp=2 },
    { .x=2, .y=0, .dest_map=MAP_VIRIDIAN_FOREST_NORTH_GATE, .dest_warp=2 },
    { .x=15, .y=47, .dest_map=MAP_VIRIDIAN_FOREST_SOUTH_GATE, .dest_warp=0 },
    { .x=16, .y=47, .dest_map=MAP_VIRIDIAN_FOREST_SOUTH_GATE, .dest_warp=0 },
    { .x=17, .y=47, .dest_map=MAP_VIRIDIAN_FOREST_SOUTH_GATE, .dest_warp=0 },
    { .x=18, .y=47, .dest_map=MAP_VIRIDIAN_FOREST_SOUTH_GATE, .dest_warp=0 },
};
"""
    body = """static const BackgroundEvent s_bg_events[] = {
    { .x=24, .y=40, .text="TRAINER TIPS\\fStay away from\\ngrassy areas to\\navoid battles!" },
    { .x=16, .y=32, .text="For poison, use\\nANTIDOTE! Get it\\nat POKeMON MARTs!" },
    { .x=26, .y=17, .text="TRAINER TIPS\\fContact PROF.OAK\\nvia PC to get your\\nPOKeDEX evaluated!" },
    { .x=4, .y=24, .text="TRAINER TIPS\\fCatch only wild\\nPOKeMON!" },
    { .x=18, .y=45, .text="TRAINER TIPS\\fWeaken POKeMON\\nbefore attempting\\ncapture!" },
    { .x=2, .y=1, .text="LEAVING VIRIDIAN\\nFOREST\\fPEWTER CITY AHEAD" },
};

static const NpcDef s_npcs[] = {
    { .x=16, .y=43, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_DOWN,
      .movement=NPC_MOVE_STAY, .text="I came here with\\nsome friends!\\fThey're out for\\nPOKeMON fights!" },
    { .x=30, .y=33, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_LEFT,
      .flags=NPCF_TRAINER, .trainer_id=TRAINER_BUG_CATCHER, .movement=NPC_MOVE_STAY,
      .trainer_party=0,
      .text="Hey! You have POKeMON!\\nCome on! Let's battle!" },
    { .x=30, .y=19, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_LEFT,
      .flags=NPCF_TRAINER, .trainer_id=TRAINER_BUG_CATCHER, .movement=NPC_MOVE_STAY,
      .trainer_party=1,
      .text="You can't jam out if\\nyou're a POKeMON trainer!" },
    { .x=2, .y=18, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_LEFT,
      .flags=NPCF_TRAINER, .trainer_id=TRAINER_BUG_CATCHER, .movement=NPC_MOVE_STAY,
      .trainer_party=2,
      .text="Hey, wait up!\\nWhat's the hurry?" },
    { .x=25, .y=11, .sprite_tile=GFX_POKEBALL_TILE_BASE, .flags=NPCF_ITEM,
      .movement=NPC_MOVE_STAY, .item_id=ITEM_ANTIDOTE, .item_flag=FLAG_VIRIDIAN_FOREST_ANTIDOTE },
    { .x=12, .y=29, .sprite_tile=GFX_POKEBALL_TILE_BASE, .flags=NPCF_ITEM,
      .movement=NPC_MOVE_STAY, .item_id=ITEM_POTION, .item_flag=FLAG_VIRIDIAN_FOREST_POTION },
    { .x=1, .y=31, .sprite_tile=GFX_POKEBALL_TILE_BASE, .flags=NPCF_ITEM,
      .movement=NPC_MOVE_STAY, .item_id=ITEM_POKE_BALL, .item_flag=FLAG_VIRIDIAN_FOREST_POKE_BALL },
    { .x=27, .y=40, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_DOWN,
      .movement=NPC_MOVE_STAY, .text="I ran out of POKeBALLs!\\nYou should carry extras!" },
};
"""
    emit_map(data / "map_viridian_forest.c", "g_map_viridian_forest", "viridian_forest", 17, 24, forest_map, warps, body, "g_tileset_forest")
    print("generated forest tileset and map")


if __name__ == "__main__":
    main()
