"""Generate the pokered Pewter City corridor and its small interiors."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REF = ROOT / "refs" / "pokered"
OUT = ROOT / "src" / "data" / "map_pewter.c"


def cells(data: bytes) -> str:
    values = [f"P(0x{value:02X})" for value in data]
    return "\n".join(
        "    " + ", ".join(values[i : i + 10]) + ","
        for i in range(0, len(values), 10)
    )


def map_layout(name: str, block_file: str, width: int, height: int, tileset: str) -> str:
    data = (REF / "maps" / block_file).read_bytes()
    return f"""static const MapCell s_{name}_cells[{width} * {height}] = {{
{cells(data)}
}};
static const MapLayout s_{name}_layout = {{
    .width = {width}, .height = {height}, .tileset = &{tileset},
    .cells = s_{name}_cells,
}};
"""


def main() -> None:
    output = [
        '#include "world.h"',
        '#include "audio.h"',
        '#include "map_ids.h"',
        '#include "gfx_npcs.h"',
        '#include "gfx_npcs_extra.h"',
        '#include "gfx_pokeball.h"',
        '#include "item.h"',
        '#include "trainer_parties.h"',
        '#include "flags.h"',
        "",
        "extern const Tileset g_tileset_overworld;",
        "extern const Tileset g_tileset_mart;",
        "extern const Tileset g_tileset_pokecenter;",
        "extern const Tileset g_tileset_gym;",
        "extern const Tileset g_tileset_gate;",
        "extern const Tileset g_tileset_house_general;",
        "#define P(m) MAPCELL_MAKE((m), 0, 0)",
        "",
        map_layout("pewter_city", "PewterCity.blk", 20, 18, "g_tileset_overworld"),
        map_layout("pewter_mart", "PewterMart.blk", 4, 4, "g_tileset_pokecenter"),
        map_layout("pewter_pokecenter", "PewterPokecenter.blk", 7, 4, "g_tileset_pokecenter"),
        map_layout("pewter_gym", "PewterGym.blk", 7, 5, "g_tileset_gym"),
        map_layout("museum_1f", "Museum1F.blk", 10, 4, "g_tileset_gate"),
        map_layout("museum_2f", "Museum2F.blk", 7, 4, "g_tileset_gate"),
        map_layout("pewter_nidoran_house", "PewterNidoranHouse.blk", 4, 4, "g_tileset_house_general"),
        map_layout("pewter_speech_house", "PewterSpeechHouse.blk", 4, 4, "g_tileset_house_general"),
        r'''static const WarpEvent s_pewter_city_warps[] = {
    { .x=14, .y=7, .dest_map=MAP_MUSEUM_1F, .dest_warp=0 },
    { .x=19, .y=5, .dest_map=MAP_MUSEUM_1F, .dest_warp=0 },
    { .x=16, .y=17, .dest_map=MAP_PEWTER_GYM, .dest_warp=0 },
    { .x=29, .y=13, .dest_map=MAP_PEWTER_NIDORAN_HOUSE, .dest_warp=0 },
    { .x=23, .y=17, .dest_map=MAP_PEWTER_MART, .dest_warp=0 },
    { .x=7, .y=29, .dest_map=MAP_PEWTER_SPEECH_HOUSE, .dest_warp=0 },
    { .x=13, .y=25, .dest_map=MAP_PEWTER_POKECENTER, .dest_warp=0 },
};
static const MapConnection s_pewter_connections[] = {
    { .direction=DIR_DOWN, .dest_map=MAP_ROUTE_2, .offset=5 },
};
static const BackgroundEvent s_pewter_bg_events[] = {
    { .x=19, .y=29, .text="TRAINER TIPS\fAny POKeMON that\ntakes part in battle,\nhowever short, earns EXP!" },
    { .x=33, .y=19, .text="NOTICE!\fThieves have been\nstealing POKeMON\nfossils at MT.MOON!\nPlease call PEWTER\nPOLICE with any info!" },
    { .x=24, .y=17, .text="PEWTER MART" },
    { .x=14, .y=25, .text="POKeMON CENTER" },
    { .x=15, .y=9, .text="PEWTER MUSEUM\nOF SCIENCE" },
    { .x=11, .y=17, .text="PEWTER CITY\nPOKeMON GYM\nLEADER: BROCK" },
    { .x=25, .y=23, .text="PEWTER CITY\fA Stone Gray City" },
};
static const NpcDef s_pewter_npcs[] = {
    { .x=8, .y=15, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_DOWN,
      .text="It's rumored that\nCLEFAIRYs came from\nthe moon!" },
    { .x=17, .y=25, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_DOWN,
      .text="There aren't many\nserious POKeMON\ntrainers here!" },
    { .x=27, .y=17, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_DOWN,
      .text="Psssst! Do you know\nwhat I'm doing?" },
    { .x=26, .y=25, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_LEFT,
      .movement=NPC_MOVE_LEFT_RIGHT, .text="I'm spraying REPEL\nto keep POKeMON\nout of my garden!" },
    { .x=35, .y=16, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_DOWN,
      .text="If you have the right\nstuff, go take on BROCK!" },
};
''',
        r'''static const WarpEvent s_pewter_mart_warps[] = {
    { .x=3, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=4 },
    { .x=4, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=4 },
};
static const WarpEvent s_pewter_pokecenter_warps[] = {
    { .x=3, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=6 },
    { .x=4, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=6 },
};
static const WarpEvent s_pewter_gym_warps[] = {
    { .x=4, .y=13, .dest_map=WARP_LAST_MAP, .dest_warp=2 },
    { .x=5, .y=13, .dest_map=WARP_LAST_MAP, .dest_warp=2 },
};
static const WarpEvent s_museum_1f_warps[] = {
    { .x=10, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=0 },
    { .x=11, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=0 },
    { .x=16, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=1 },
    { .x=17, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=1 },
};
static const WarpEvent s_pewter_nidoran_house_warps[] = {
    { .x=2, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=3 },
    { .x=3, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=3 },
};
static const WarpEvent s_pewter_speech_house_warps[] = {
    { .x=2, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=5 },
    { .x=3, .y=7, .dest_map=WARP_LAST_MAP, .dest_warp=5 },
};

static const NpcDef s_pewter_mart_npcs[] = {
    { .x=0, .y=5, .sprite_tile=GFX_SCIENTIST_TILE_BASE, .facing=DIR_RIGHT, .text="Welcome to the\nPOKeMON MART!" },
    { .x=3, .y=3, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_DOWN, .text="You can buy items\nfor your POKeMON." },
};
static const NpcDef s_pewter_center_npcs[] = {
    { .x=3, .y=1, .sprite_tile=GFX_GIRL_TILE_BASE, .facing=DIR_DOWN, .text="Welcome to the\nPOKeMON CENTER." },
    { .x=1, .y=3, .sprite_tile=GFX_GIRL_TILE_BASE, .facing=DIR_DOWN, .text="We hope to see you\nagain!" },
};
static const NpcDef s_pewter_gym_npcs[] = {
    { .x=4, .y=1, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_DOWN, .text="I'm BROCK's first\nopponent!" },
    { .x=7, .y=10, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_DOWN, .text="Yo! Champ in making!" },
};
static const NpcDef s_museum_npcs[] = {
    { .x=12, .y=4, .sprite_tile=GFX_SCIENTIST_TILE_BASE, .facing=DIR_LEFT, .text="Welcome to the\nPEWTER MUSEUM." },
    { .x=1, .y=4, .sprite_tile=GFX_GAMBLER_TILE_BASE, .facing=DIR_DOWN, .text="These fossils are\nfrom MT.MOON." },
};
static const NpcDef s_nidoran_npcs[] = {
    { .x=4, .y=5, .sprite_tile=GFX_GIRL_TILE_BASE, .facing=DIR_LEFT, .text="NIDORAN!" },
    { .x=3, .y=5, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_RIGHT, .text="I love my NIDORAN." },
};
static const NpcDef s_speech_npcs[] = {
    { .x=2, .y=3, .sprite_tile=GFX_GAMBLER_TILE_BASE, .facing=DIR_RIGHT, .text="Have you heard of\nCLEFAIRY?" },
    { .x=4, .y=5, .sprite_tile=GFX_YOUNGSTER_TILE_BASE, .facing=DIR_DOWN, .text="I want to see\nCLEFAIRY." },
};
''',
        r'''const MapHeader g_map_pewter_city = {
    .map_id=MAP_PEWTER_CITY, .name="Pewter City", .layout=&s_pewter_city_layout,
    .warps=s_pewter_city_warps, .warp_count=ARRAY_COUNT(s_pewter_city_warps),
    .npcs=s_pewter_npcs, .npc_count=ARRAY_COUNT(s_pewter_npcs),
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY, .bg_events=s_pewter_bg_events,
    .bg_event_count=ARRAY_COUNT(s_pewter_bg_events), .connections=s_pewter_connections,
    .connection_count=ARRAY_COUNT(s_pewter_connections),
};
const MapHeader g_map_pewter_mart = {
    .map_id=MAP_PEWTER_MART, .name="Pewter Mart", .layout=&s_pewter_mart_layout,
    .warps=s_pewter_mart_warps, .warp_count=ARRAY_COUNT(s_pewter_mart_warps),
    .npcs=s_pewter_mart_npcs, .npc_count=ARRAY_COUNT(s_pewter_mart_npcs),
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY,
};
const MapHeader g_map_pewter_pokecenter = {
    .map_id=MAP_PEWTER_POKECENTER, .name="Pewter Pokemon Center", .layout=&s_pewter_pokecenter_layout,
    .warps=s_pewter_pokecenter_warps, .warp_count=ARRAY_COUNT(s_pewter_pokecenter_warps),
    .npcs=s_pewter_center_npcs, .npc_count=ARRAY_COUNT(s_pewter_center_npcs),
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY,
};
const MapHeader g_map_pewter_gym = {
    .map_id=MAP_PEWTER_GYM, .name="Pewter Gym", .layout=&s_pewter_gym_layout,
    .warps=s_pewter_gym_warps, .warp_count=ARRAY_COUNT(s_pewter_gym_warps),
    .npcs=s_pewter_gym_npcs, .npc_count=ARRAY_COUNT(s_pewter_gym_npcs),
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY,
};
const MapHeader g_map_museum_1f = {
    .map_id=MAP_MUSEUM_1F, .name="Pewter Museum", .layout=&s_museum_1f_layout,
    .warps=s_museum_1f_warps, .warp_count=ARRAY_COUNT(s_museum_1f_warps),
    .npcs=s_museum_npcs, .npc_count=ARRAY_COUNT(s_museum_npcs),
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY,
};
const MapHeader g_map_pewter_nidoran_house = {
    .map_id=MAP_PEWTER_NIDORAN_HOUSE, .name="Pewter Nidoran House", .layout=&s_pewter_nidoran_house_layout,
    .warps=s_pewter_nidoran_house_warps, .warp_count=ARRAY_COUNT(s_pewter_nidoran_house_warps),
    .npcs=s_nidoran_npcs, .npc_count=ARRAY_COUNT(s_nidoran_npcs),
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY,
};
const MapHeader g_map_pewter_speech_house = {
    .map_id=MAP_PEWTER_SPEECH_HOUSE, .name="Pewter Speech House", .layout=&s_pewter_speech_house_layout,
    .warps=s_pewter_speech_house_warps, .warp_count=ARRAY_COUNT(s_pewter_speech_house_warps),
    .npcs=s_speech_npcs, .npc_count=ARRAY_COUNT(s_speech_npcs),
    .music_id=AUDIO_MUSIC_VIRIDIAN_CITY,
};

#undef P
''',
    ]
    OUT.write_text("\n".join(output), encoding="utf-8")
    print(f"generated {OUT}")


if __name__ == "__main__":
    main()
