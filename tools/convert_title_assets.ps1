param(
    [string]$ReferenceDir = "E:\pokemon recomp\refs\pokered\gfx\title",
    [string]$PokemonFrontDir = "E:\pokemon recomp\refs\pokered\gfx\pokemon\front",
    [string]$OutputDir = "E:\pokemon recomp\src\data",
    [string]$TempDir = "E:\pokemon recomp\tmp\title-convert"
)

New-Item -ItemType Directory -Force -Path $TempDir,$OutputDir | Out-Null

$assets = @(
    @{ Name = "title_logo"; Input = "pokemon_logo.png"; Width = 128; Height = 56 },
    @{ Name = "title_version"; Input = "red_version.png"; Width = 80; Height = 8 },
    @{ Name = "title_player"; Input = "player.png"; Width = 40; Height = 56 },
    @{ Name = "title_charmander"; Input = (Join-Path $PokemonFrontDir "charmander.png"); Width = 40; Height = 40; Absolute = $true },
    @{ Name = "intro_oak"; Input = "E:\pokemon recomp\refs\pokered\gfx\trainers\prof.oak.png"; Width = 56; Height = 56; Absolute = $true },
    @{ Name = "intro_rival"; Input = "E:\pokemon recomp\refs\pokered\gfx\trainers\rival1.png"; Width = 56; Height = 56; Absolute = $true },
    @{ Name = "intro_red"; Input = "E:\pokemon recomp\refs\pokered\gfx\player\red.png"; Width = 56; Height = 56; Absolute = $true },
    @{ Name = "intro_red_battle"; Input = "E:\pokemon recomp\refs\pokered\gfx\player\redb.png"; Width = 32; Height = 32; Absolute = $true },
    @{ Name = "intro_nidorino"; Input = "E:\pokemon recomp\refs\pokered\gfx\intro\red_nidorino_1.png"; Width = 48; Height = 48; Absolute = $true }
)

$header = @(
    "#pragma once",
    '#include "types.h"',
    "",
    "extern const u32 g_title_logo_tiles[];",
    "extern const u32 g_title_version_tiles[];",
    "extern const u32 g_title_player_tiles[];",
    "extern const u32 g_title_charmander_tiles[];",
    "extern const u32 g_intro_oak_tiles[];",
    "extern const u32 g_intro_rival_tiles[];",
    "extern const u32 g_intro_red_tiles[];",
    "extern const u32 g_intro_red_battle_tiles[];",
    "extern const u32 g_intro_nidorino_tiles[];",
    ""
)
$source = @(
    '#include "types.h"',
    '#include "gfx_title.h"',
    ""
)

foreach ($asset in $assets) {
    $input = if ($asset.Absolute) { $asset.Input } else { Join-Path $ReferenceDir $asset.Input }
    $raw = Join-Path $TempDir ($asset.Name + ".raw")
    & ffmpeg -loglevel error -i $input -f rawvideo -pix_fmt gray $raw -y
    if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed for $input" }

    $pixels = [IO.File]::ReadAllBytes($raw)
    $tilesWide = [int]($asset.Width / 8)
    $tilesHigh = [int]($asset.Height / 8)
    $words = New-Object System.Collections.Generic.List[UInt32]

    for ($ty = 0; $ty -lt $tilesHigh; $ty++) {
        for ($tx = 0; $tx -lt $tilesWide; $tx++) {
            for ($py = 0; $py -lt 8; $py++) {
                [UInt32]$word = 0
                for ($px = 0; $px -lt 8; $px++) {
                    $value = $pixels[(($ty * 8 + $py) * $asset.Width) + ($tx * 8 + $px)]
                    $color = switch ($value) {
                        0   { 1 }
                        85  { 2 }
                        170 { 3 }
                        default { 0 }
                    }
                    $word = $word -bor ([UInt32]$color -shl (4 * $px))
                }
                $words.Add($word)
            }
        }
    }

    $symbol = "g_" + $asset.Name + "_tiles"
    $source += "const u32 $symbol[$($words.Count)] = {"
    for ($i = 0; $i -lt $words.Count; $i += 8) {
        $end = [Math]::Min($i + 8, $words.Count)
        $row = for ($j = $i; $j -lt $end; $j++) { "0x{0:X8}" -f $words[$j] }
        $source += "    " + ($row -join ", ") + ","
    }
    $source += "};"
    $source += ""
}

Set-Content -LiteralPath (Join-Path $OutputDir "gfx_title.h") -Value $header -Encoding ascii
Set-Content -LiteralPath (Join-Path $OutputDir "gfx_title.c") -Value $source -Encoding ascii
