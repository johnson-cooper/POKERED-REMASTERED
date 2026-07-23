param(
    [string]$ReferenceDir = "E:\pokemon recomp\refs\pokered\gfx",
    [string]$OutputDir = "E:\pokemon recomp\src\data",
    [string]$TempDir = "E:\pokemon recomp\tmp\house-convert"
)

New-Item -ItemType Directory -Force -Path $TempDir,$OutputDir | Out-Null

$png = Join-Path $ReferenceDir "tilesets\house.png"
$bst = Join-Path $ReferenceDir "blocksets\house.bst"
$raw = Join-Path $TempDir "house.raw"
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
$gfx.Add('#include "gfx_house_general.h"')
$gfx.Add('')
$gfx.Add('const u32 g_house_general_tiles[768] = {')
for ($i = 0; $i -lt $words.Count; $i += 8) {
    $row = for ($j = $i; $j -lt ($i + 8); $j++) { "0x{0:X8}" -f $words[$j] }
    $gfx.Add("    " + ($row -join ", ") + ",")
}
$gfx.Add('};')
$gfx.Add('')
$gfx.Add('const unsigned short g_house_general_pal[16] = {')
$gfx.Add('    0x2108, 0x2108, 0x2108, 0x2108, 0x2108, 0x2108, 0x2108, 0x2D4A,')
$gfx.Add('    0x2D4A, 0x3DEF, 0x3DEF, 0x3DEF, 0x3DEF, 0x4A52, 0x4A52, 0x56B5')
$gfx.Add('};')
Set-Content -LiteralPath (Join-Path $OutputDir "gfx_house_general.c") -Value $gfx -Encoding ascii

$header = @(
    '#pragma once',
    '#include "types.h"',
    '',
    'extern const u32 g_house_general_tiles[];',
    'extern const unsigned short g_house_general_pal[16];',
    ''
)
Set-Content -LiteralPath (Join-Path $OutputDir "gfx_house_general.h") -Value $header -Encoding ascii

$blocks = [IO.File]::ReadAllBytes($bst)
$tileset = New-Object System.Collections.Generic.List[string]
$tileset.Add('#include "world.h"')
$tileset.Add('#include "gfx_house_general.h"')
$tileset.Add('')
$tileset.Add('#define ZERO16 {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}')
$tileset.Add('')
$tileset.Add('static const Metatile s_house_general_metatiles[35] = {')
for ($block = 0; $block -lt 35; $block++) {
    $vals = $blocks[($block * 16)..($block * 16 + 15)] | ForEach-Object { "0x{0:X2}" -f $_ }
    $tileset.Add("    [$block] = { .bottom={" + ($vals -join ',') + "}, .top=ZERO16, .palettes=ZERO16, .top_palettes=ZERO16 },")
}
$tileset.Add('};')
$tileset.Add('')
$tileset.Add('static const u8 s_house_general_collision[] = { 0x01,0x12,0x14,0x28,0x32,0x37,0x44,0x54,0x5C };')
$tileset.Add('')
$tileset.Add('const Tileset g_tileset_house_general = {')
$tileset.Add('    .tiles=(const u32 *)g_house_general_tiles, .tile_count=96,')
$tileset.Add('    .palettes=g_house_general_pal, .palette_count=1,')
$tileset.Add('    .metatiles=s_house_general_metatiles, .metatile_count=35,')
$tileset.Add('    .use_cell_collision=TRUE, .collision_tiles=s_house_general_collision,')
$tileset.Add('    .collision_tile_count=ARRAY_COUNT(s_house_general_collision),')
$tileset.Add('};')
Set-Content -LiteralPath (Join-Path $OutputDir "tileset_house_general.c") -Value $tileset -Encoding ascii
