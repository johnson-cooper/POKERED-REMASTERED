"""Generate the complete overworld block table needed by Route 1."""
from pathlib import Path

src = Path("refs/pokered/gfx/blocksets/overworld.bst").read_bytes()
out = Path("src/data/route1_metatiles.c")

lines = [
    '#include "world.h"',
    '#include "gba.h"',
    '#include "gfx_overworld.h"',
    '#include "gfx_overworld_color.h"',
    '#include "overworld_palette_map.h"',
    '',
    'static const Metatile s_route1_metatiles[120] = {',
]
for block in range(120):
    tiles = list(src[block * 16:block * 16 + 16])
    tile_text = ", ".join(f"0x{tile:02X}" for tile in tiles)
    lines.append(f"    [{block}] = {{ .bottom={{ {tile_text} }},")
    lines.append(f"        .top={{ 0 }}, .palettes={{ 0 }}, .top_palettes={{ 0 }} }},")
lines += [
    '};',
    '',
    'const Tileset g_tileset_route1 = {',
    '    .tiles = g_overworld_color_tiles,',
    '    .overlay_tiles = g_overworld_tiles,',
    '    .tile_count = 96,',
    '    .palettes = g_overworld_palette_colors,',
    '    .palette_count = 8,',
    '    .palette_profile = &g_overworld_palette_profile,',
    '    .tile_palette_map = g_overworld_tile_palette_map,',
    '    .metatiles = s_route1_metatiles,',
    '    .metatile_count = 120,',
    '};',
]
out.write_text("\n".join(lines) + "\n", encoding="ascii")
