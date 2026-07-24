param(
    [string]$ReferenceDir = "E:\pokemon recomp\refs\pokered\gfx",
    [string]$OutputDir = "E:\pokemon recomp\src\data",
    [string]$TempDir = "E:\pokemon recomp\tmp\party-icons"
)

$icons = @(
    # The first frame is the normal party-menu frame; the second is the
    # animated frame used by ICONOFFSET in pokered's OAM updater.
    @{ Symbol = "monster"; File = "sprites\monster.png"; Width = 16; Frames = @(12, 0) },
    @{ Symbol = "fairy"; File = "sprites\fairy.png"; Width = 16; Frames = @(12, 0) },
    @{ Symbol = "bird"; File = "sprites\bird.png"; Width = 16; Frames = @(12, 0) },
    @{ Symbol = "water"; File = "sprites\seel.png"; Width = 16; Frames = @(0, 12) },
    # The small icon sheets are four 8x8 tiles tall. Frame 2 begins at tile 2
    # ($20 bytes), matching BugIconFrame2/PlantIconFrame2 in pokered.
    @{ Symbol = "bug"; File = "icons\bug.png"; Width = 8; Frames = @(2, 0) },
    @{ Symbol = "grass"; File = "icons\plant.png"; Width = 8; Frames = @(2, 0) },
    @{ Symbol = "snake"; File = "icons\snake.png"; Width = 8; Frames = @(0, 2) },
    @{ Symbol = "quadruped"; File = "icons\quadruped.png"; Width = 8; Frames = @(0, 2) }
)

New-Item -ItemType Directory -Force -Path $TempDir,$OutputDir | Out-Null
$lines = New-Object System.Collections.Generic.List[string]
$lines.Add('#include "gfx_party_icons.h"')
$lines.Add('')

foreach ($icon in $icons) {
    $raw = Join-Path $TempDir ($icon.Symbol + ".raw")
    & ffmpeg -loglevel error -i (Join-Path $ReferenceDir $icon.File) -f rawvideo -pix_fmt gray $raw -y
    if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed for $($icon.File)" }
    $pixels = [IO.File]::ReadAllBytes($raw)
    $tileWidth = [int]($icon.Width / 8)
    $allWords = New-Object System.Collections.Generic.List[UInt32]
    foreach ($offset in $icon.Frames) {
        $words = New-Object System.Collections.Generic.List[UInt32]
        $count = if ($icon.Width -eq 8) { 2 } else { 4 }
        for ($tile = $offset; $tile -lt ($offset + $count); $tile++) {
            $tx = $tile % $tileWidth
            $ty = [int]($tile / $tileWidth)
            for ($py = 0; $py -lt 8; $py++) {
                [UInt32]$word = 0
                for ($px = 0; $px -lt 8; $px++) {
                    $value = $pixels[(($ty * 8 + $py) * $icon.Width) + ($tx * 8 + $px)]
                    # The pokered icon PNGs use white as the transparent paper
                    # color and black as the darkest sprite shade.
                    $color = switch ($value) { 255 { 0 }; 170 { 1 }; 85 { 2 }; default { 3 } }
                    $word = $word -bor ([UInt32]$color -shl (4 * $px))
                }
                $words.Add($word)
            }
        }
        # Keep the source tiles in the first half of each 32-word frame. The
        # renderer mirrors these into the right half just like pokered OAM.
        if ($icon.Width -eq 8) {
            $sourceTiles = New-Object System.Collections.Generic.List[UInt32]
            foreach ($word in $words) { $sourceTiles.Add($word) }
            $words = New-Object System.Collections.Generic.List[UInt32]
            foreach ($word in $sourceTiles[0..7]) { $words.Add($word) }
            for ($i = 0; $i -lt 8; $i++) { $words.Add([UInt32]0) }
            foreach ($word in $sourceTiles[8..15]) { $words.Add($word) }
            for ($i = 0; $i -lt 8; $i++) { $words.Add([UInt32]0) }
        }
        foreach ($word in $words) { $allWords.Add($word) }
    }
    $lines.Add("const u32 g_party_icon_$($icon.Symbol)_tiles[$($allWords.Count)] = {")
    for ($i = 0; $i -lt $allWords.Count; $i += 8) {
        $row = for ($j = $i; $j -lt [Math]::Min($i + 8, $allWords.Count); $j++) { "0x{0:X8}" -f $allWords[$j] }
        $lines.Add("    " + ($row -join ", ") + ",")
    }
    $lines.Add('};')
    $lines.Add('')
}
Set-Content -LiteralPath (Join-Path $OutputDir "gfx_party_icons.c") -Value $lines -Encoding ascii

$header = @('#pragma once', '#include "types.h"', '')
foreach ($icon in $icons) { $header += "extern const u32 g_party_icon_$($icon.Symbol)_tiles[];" }
$header += ''
Set-Content -LiteralPath (Join-Path $OutputDir "gfx_party_icons.h") -Value $header -Encoding ascii
