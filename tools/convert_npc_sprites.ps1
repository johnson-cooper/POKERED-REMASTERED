param(
    [string]$ReferenceDir = "E:\pokemon recomp\refs\pokered\gfx\sprites",
    [string]$OutputDir = "E:\pokemon recomp\src\data",
    [string]$TempDir = "E:\pokemon recomp\tmp\npc-convert"
)

$sprites = @(
    @{ Symbol = "youngster"; File = (Join-Path $ReferenceDir "youngster.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "gambler"; File = (Join-Path $ReferenceDir "gambler.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "gambler_asleep"; File = (Join-Path $ReferenceDir "gambler_asleep.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "girl";    File = (Join-Path $ReferenceDir "girl.png");    Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "fisher";  File = (Join-Path $ReferenceDir "fisher.png");  Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "daisy";   File = (Join-Path $ReferenceDir "daisy.png");   Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "mom";     File = (Join-Path $ReferenceDir "mom.png");     Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "scientist"; File = (Join-Path $ReferenceDir "scientist.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "cooltrainer_f"; File = (Join-Path $ReferenceDir "cooltrainer_f.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "cooltrainer_m"; File = (Join-Path $ReferenceDir "cooltrainer_m.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "super_nerd"; File = (Join-Path $ReferenceDir "super_nerd.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "clerk"; File = (Join-Path $ReferenceDir "clerk.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "nurse"; File = (Join-Path $ReferenceDir "nurse.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "fairy"; File = (Join-Path $ReferenceDir "fairy.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "gentleman"; File = (Join-Path $ReferenceDir "gentleman.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "gramps"; File = (Join-Path $ReferenceDir "gramps.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "brunette_girl"; File = (Join-Path $ReferenceDir "brunette_girl.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "hiker"; File = (Join-Path $ReferenceDir "hiker.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "little_boy"; File = (Join-Path $ReferenceDir "little_boy.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "little_girl"; File = (Join-Path $ReferenceDir "little_girl.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "middle_aged_man"; File = (Join-Path $ReferenceDir "middle_aged_man.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "monster"; File = (Join-Path $ReferenceDir "monster.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "old_amber"; File = (Join-Path $ReferenceDir "old_amber.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "gym_guide"; File = (Join-Path $ReferenceDir "gym_guide.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "link_receptionist"; File = (Join-Path $ReferenceDir "link_receptionist.png"); Width = 16; Height = 96; Frames = 6 },
    @{ Symbol = "pokedex_overworld"; File = (Join-Path $ReferenceDir "pokedex.png"); Width = 16; Height = 16; Frames = 1 }
)

New-Item -ItemType Directory -Force -Path $TempDir,$OutputDir | Out-Null
$lines = New-Object System.Collections.Generic.List[string]
$lines.Add('#include "gfx_npcs_extra.h"')
$lines.Add('')

foreach ($sprite in $sprites) {
    $raw = Join-Path $TempDir ($sprite.Symbol + ".raw")
    & ffmpeg -loglevel error -i $sprite.File -f rawvideo -pix_fmt gray $raw -y
    if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed for $($sprite.File)" }

    $pixels = [IO.File]::ReadAllBytes($raw)
    $words = New-Object System.Collections.Generic.List[UInt32]
    $tile_rows = [int]($sprite.Height / 8)
    for ($ty = 0; $ty -lt $tile_rows; $ty++) {
        for ($tx = 0; $tx -lt 2; $tx++) {
            for ($py = 0; $py -lt 8; $py++) {
                [UInt32]$word = 0
                for ($px = 0; $px -lt 8; $px++) {
                    $value = $pixels[(($ty * 8 + $py) * $sprite.Width) + ($tx * 8 + $px)]
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

    $word_count = [int]($sprite.Width / 8) * $tile_rows * 8
    $lines.Add("const u32 g_$($sprite.Symbol)_tiles[$word_count] = {")
    for ($i = 0; $i -lt $words.Count; $i += 8) {
        $row = for ($j = $i; $j -lt ($i + 8); $j++) { "0x{0:X8}" -f $words[$j] }
        $lines.Add("    " + ($row -join ", ") + ",")
    }
    $lines.Add('};')
    $lines.Add('')
}

Set-Content -LiteralPath (Join-Path $OutputDir "gfx_npcs_extra.c") -Value $lines -Encoding ascii

$header = @(
    '#pragma once',
    '#include "types.h"',
    '',
    'extern const u32 g_youngster_tiles[];',
    'extern const u32 g_gambler_tiles[];',
    'extern const u32 g_gambler_asleep_tiles[];',
    'extern const u32 g_girl_tiles[];',
    'extern const u32 g_fisher_tiles[];',
    'extern const u32 g_daisy_tiles[];',
    'extern const u32 g_mom_tiles[];',
    'extern const u32 g_scientist_tiles[];',
    'extern const u32 g_cooltrainer_f_tiles[];',
    'extern const u32 g_cooltrainer_m_tiles[];',
    'extern const u32 g_super_nerd_tiles[];',
    'extern const u32 g_clerk_tiles[];',
    'extern const u32 g_nurse_tiles[];',
    'extern const u32 g_fairy_tiles[];',
    'extern const u32 g_gentleman_tiles[];',
    'extern const u32 g_gramps_tiles[];',
    'extern const u32 g_brunette_girl_tiles[];',
    'extern const u32 g_hiker_tiles[];',
    'extern const u32 g_little_boy_tiles[];',
    'extern const u32 g_little_girl_tiles[];',
    'extern const u32 g_middle_aged_man_tiles[];',
    'extern const u32 g_monster_tiles[];',
    'extern const u32 g_old_amber_tiles[];',
    'extern const u32 g_gym_guide_tiles[];',
    'extern const u32 g_link_receptionist_tiles[];',
    'extern const u32 g_pokedex_overworld_tiles[];',
    ''
)
Set-Content -LiteralPath (Join-Path $OutputDir "gfx_npcs_extra.h") -Value $header -Encoding ascii
