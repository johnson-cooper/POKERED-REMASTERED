"""Generate the complete overworld block table needed by Route 1."""
from pathlib import Path

src = Path("refs/pokered/gfx/blocksets/overworld.bst").read_bytes()
out = Path("src/data/route1_metatiles.c")

def palette(tile):
    if tile == 0x14:
        return 6
    if tile in {0x20, 0x36, 0x54, 0x0E, 0x2E, 0x2F, 0x37,
                0x46, 0x47, 0x55, 0x56, 0x57}:
        return 5
    if tile in {0x2A, 0x2B, 0x3A, 0x3B}:
        return 4
    if tile in {0x0A, 0x0F, 0x1A, 0x22, 0x25, 0x26, 0x2C, 0x33, 0x4E}:
        return 1
    return 0

lines = [
    '#include "world.h"',
    '#include "gba.h"',
    '#include "gfx_overworld.h"',
    '',
    'static const Metatile s_route1_metatiles[120] = {',
]
for block in range(120):
    tiles = list(src[block * 16:block * 16 + 16])
    pals = [palette(tile) for tile in tiles]
    tile_text = ", ".join(f"0x{tile:02X}" for tile in tiles)
    pal_text = ", ".join(str(pal) for pal in pals)
    lines.append(f"    [{block}] = {{ .bottom={{ {tile_text} }},")
    lines.append(f"        .top={{ 0 }}, .palettes={{ {pal_text} }}, .top_palettes={{ 0 }} }},")
lines += [
    '};',
    '',
    '#define E 0x0000',
    'static const u16 s_route1_palettes[9 * 16] = {',
    '    RGB15(1,6,0), RGB15(1,6,0),E,E,E, RGB15(7,18,4), E,E,E,E, RGB15(13,25,8), E,E,E,E, RGB15(21,30,13),',
    '    RGB15(8,6,3), RGB15(8,6,3),E,E,E, RGB15(18,16,10), E,E,E,E, RGB15(24,22,16), E,E,E,E, RGB15(30,28,22),',
    '    RGB15(6,5,3), RGB15(6,5,3),E,E,E, RGB15(15,13,9), E,E,E,E, RGB15(23,21,16), E,E,E,E, RGB15(26,24,19),',
    '    RGB15(6,5,3), RGB15(6,5,3),E,E,E, RGB15(15,13,9), E,E,E,E, RGB15(23,21,16), E,E,E,E, RGB15(26,24,19),',
    '    RGB15(0,2,0), RGB15(0,2,0),E,E,E, RGB15(4,10,2), E,E,E,E, RGB15(8,18,5), E,E,E,E, RGB15(14,24,9),',
    '    RGB15(5,3,1), RGB15(5,3,1),E,E,E, RGB15(13,9,4), E,E,E,E, RGB15(22,16,8), E,E,E,E, RGB15(30,24,14),',
    '    RGB15(0,2,7), RGB15(0,2,7),E,E,E, RGB15(2,7,16), E,E,E,E, RGB15(5,13,23), E,E,E,E, RGB15(9,18,28),',
    '    RGB15(8,7,1), RGB15(8,7,1),E,E,E, RGB15(18,16,5), E,E,E,E, RGB15(27,24,10), E,E,E,E, RGB15(31,31,22),',
    '    RGB15(0,5,1), RGB15(0,5,1),E,E,E, RGB15(2,10,18), E,E,E,E, RGB15(5,16,26), E,E,E,E, RGB15(14,24,31),',
    '};',
    '#undef E',
    '',
    '',
    'const Tileset g_tileset_route1 = {',
    '    .tiles = g_overworld_tiles,',
    '    .tile_count = 96,',
    '    .palettes = s_route1_palettes,',
    '    .palette_count = 9,',
    '    .metatiles = s_route1_metatiles,',
    '    .metatile_count = 120,',
    '};',
]
out.write_text("\n".join(lines) + "\n", encoding="ascii")
