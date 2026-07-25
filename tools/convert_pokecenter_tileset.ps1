param(
    [string]$ReferenceDir = "E:\pokemon recomp\refs\pokered\gfx",
    [string]$OutputDir = "E:\pokemon recomp\src\data",
    [string]$TempDir = "E:\pokemon recomp\tmp\pokecenter-convert"
)

New-Item -ItemType Directory -Force -Path $TempDir,$OutputDir | Out-Null

$png = Join-Path $ReferenceDir "tilesets\pokecenter.png"
$bst = Join-Path $ReferenceDir "blocksets\pokecenter.bst"
$raw = Join-Path $TempDir "pokecenter.raw"
& ffmpeg -loglevel error -i $png -f rawvideo -pix_fmt gray $raw -y
if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed for $png" }

$pixels = [IO.File]::ReadAllBytes($raw)
$words = New-Object System.Collections.Generic.List[UInt32]
for ($ty = 0; $ty -lt 6; $ty++) {
    for ($tx = 0; $tx -lt 16; $tx++) {
        for ($py = 0; $py -lt 8; $py++) {
            [UInt32]$word = 0
            for ($px = 0; $px -lt 8; $px++) {
                $value = $pixels[(($ty * 8 + $py) * 128) + ($tx * 8 + $px)]
                $color = switch ($value) { 0 { 0 } 85 { 5 } 170 { 10 } default { 15 } }
                $word = $word -bor ([UInt32]$color -shl (4 * $px))
            }
            $words.Add($word)
        }
    }
}

$gfx = New-Object System.Collections.Generic.List[string]
$gfx.Add('#include "gfx_pokecenter.h"')
$gfx.Add('')
$gfx.Add('const u32 g_pokecenter_tiles[768] = {')
for ($i = 0; $i -lt $words.Count; $i += 8) {
    $row = for ($j = $i; $j -lt ($i + 8); $j++) { "0x{0:X8}" -f $words[$j] }
    $gfx.Add("    " + ($row -join ", ") + ",")
}
$gfx.Add('};')
$gfx.Add('')
Set-Content -LiteralPath (Join-Path $OutputDir "gfx_pokecenter.c") -Value $gfx -Encoding ascii

$header = @(
    '#pragma once',
    '#include "types.h"',
    '',
    'extern const u32 g_pokecenter_tiles[];',
    ''
)
Set-Content -LiteralPath (Join-Path $OutputDir "gfx_pokecenter.h") -Value $header -Encoding ascii

$blocks = [IO.File]::ReadAllBytes($bst)
$tileset = New-Object System.Collections.Generic.List[string]
$tileset.Add('#include "world.h"')
$tileset.Add('#include "gfx_pokecenter.h"')
$tileset.Add('#include "indoor_palette.h"')
$tileset.Add('')
$tileset.Add('#define ZERO16 {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}')
$tileset.Add('')
$tileset.Add('static const Metatile s_pokecenter_metatiles[37] = {')
for ($block = 0; $block -lt 37; $block++) {
    $vals = $blocks[($block * 16)..($block * 16 + 15)] | ForEach-Object { "0x{0:X2}" -f $_ }
    $tileset.Add("    [$block] = { .bottom={" + ($vals -join ',') + "}, .top=ZERO16, .palettes=ZERO16, .top_palettes=ZERO16 },")
}
$tileset.Add('};')
$tileset.Add('')
$tileset.Add('static const u8 s_pokecenter_collision[] = { 0x11,0x1A,0x1C,0x3C,0x5E };')
$tileset.Add('')
$tileset.Add('const Tileset g_tileset_pokecenter = {')
$tileset.Add('    .tiles=(const u32 *)g_pokecenter_tiles, .tile_count=96,')
$tileset.Add('    .palette_profile=&g_indoor_palette_profile, .tile_palette_map=g_house_tile_palette_map,')
$tileset.Add('    .metatiles=s_pokecenter_metatiles, .metatile_count=37,')
$tileset.Add('    .use_cell_collision=TRUE, .collision_tiles=s_pokecenter_collision,')
$tileset.Add('    .collision_tile_count=ARRAY_COUNT(s_pokecenter_collision),')
$tileset.Add('};')
Set-Content -LiteralPath (Join-Path $OutputDir "tileset_pokecenter.c") -Value $tileset -Encoding ascii
