param(
    [string]$InputFile = "E:\pokemon recomp\refs\pokered\gfx\sprites\red.png",
    [string]$OutputDir = "E:\pokemon recomp\src\data",
    [string]$TempDir = "E:\pokemon recomp\tmp\player-convert"
)

New-Item -ItemType Directory -Force -Path $TempDir,$OutputDir | Out-Null
$raw = Join-Path $TempDir "red.raw"
& ffmpeg -loglevel error -i $InputFile -f rawvideo -pix_fmt gray $raw -y
if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed for $InputFile" }

$pixels = [IO.File]::ReadAllBytes($raw)
$width = 16
$height = 96
$words = New-Object System.Collections.Generic.List[UInt32]

for ($ty = 0; $ty -lt ($height / 8); $ty++) {
    for ($tx = 0; $tx -lt ($width / 8); $tx++) {
        for ($py = 0; $py -lt 8; $py++) {
            [UInt32]$word = 0
            for ($px = 0; $px -lt 8; $px++) {
                $value = $pixels[(($ty * 8 + $py) * $width) + ($tx * 8 + $px)]
                $color = switch ($value) {
                    0   { 0 }
                    85  { 5 }
                    170 { 10 }
                    default { 15 }
                }
                $word = $word -bor ([UInt32]$color -shl (4 * $px))
            }
            $words.Add($word)
        }
    }
}

$header = @(
    "#pragma once",
    '#include "types.h"',
    "",
    "extern const u32 g_player_tiles[];",
    "extern const u32 g_player_tile_count;",
    ""
)
$source = @(
    '#include "gba.h"',
    '#include "gfx_player.h"',
    "",
    "const u32 g_player_tiles[$($words.Count)] = {"
)
for ($i = 0; $i -lt $words.Count; $i += 8) {
    $end = [Math]::Min($i + 8, $words.Count)
    $row = for ($j = $i; $j -lt $end; $j++) { "0x{0:X8}" -f $words[$j] }
    $source += "    " + ($row -join ", ") + ","
}
$source += "};"
$source += "const u32 g_player_tile_count = $($words.Count / 8);"
$source += ""

Set-Content -LiteralPath (Join-Path $OutputDir "gfx_player.h") -Value $header -Encoding ascii
Set-Content -LiteralPath (Join-Path $OutputDir "gfx_player.c") -Value $source -Encoding ascii
