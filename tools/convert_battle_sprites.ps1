param(
    [string]$ReferenceDir = "E:\pokemon recomp\refs\pokered\gfx\pokemon",
    [string]$OutputFile = "E:\pokemon recomp\src\data\gfx_battle_sprites.c"
)

$sprites = @(
    @{ Symbol = "bulbasaur_front"; File = (Join-Path $ReferenceDir "front\bulbasaur.png"); Width = 40; Height = 40 },
    @{ Symbol = "charmander_front"; File = (Join-Path $ReferenceDir "front\charmander.png"); Width = 40; Height = 40 },
    @{ Symbol = "squirtle_front"; File = (Join-Path $ReferenceDir "front\squirtle.png"); Width = 40; Height = 40 },
    @{ Symbol = "pidgey_front"; File = (Join-Path $ReferenceDir "front\pidgey.png"); Width = 40; Height = 40 },
    @{ Symbol = "rattata_front"; File = (Join-Path $ReferenceDir "front\rattata.png"); Width = 40; Height = 40 },
    @{ Symbol = "bulbasaur_back"; File = (Join-Path $ReferenceDir "back\bulbasaurb.png"); Width = 32; Height = 32 },
    @{ Symbol = "charmander_back"; File = (Join-Path $ReferenceDir "back\charmanderb.png"); Width = 32; Height = 32 },
    @{ Symbol = "squirtle_back"; File = (Join-Path $ReferenceDir "back\squirtleb.png"); Width = 32; Height = 32 },
    @{ Symbol = "pidgey_back"; File = (Join-Path $ReferenceDir "back\pidgeyb.png"); Width = 32; Height = 32 },
    @{ Symbol = "rattata_back"; File = (Join-Path $ReferenceDir "back\rattatab.png"); Width = 32; Height = 32 }
)

$tempDir = Join-Path ([IO.Path]::GetTempPath()) "pokered-battle-sprites"
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null
$lines = New-Object System.Collections.Generic.List[string]
$lines.Add('#include "gfx_battle_sprites.h"')
$lines.Add('')
$lines.Add('#define BATTLE_WORDS (BATTLE_SPRITE_WORDS)')
$lines.Add('')

foreach ($sprite in $sprites) {
    $raw = Join-Path $tempDir ($sprite.Symbol + ".raw")
    & ffmpeg -loglevel error -i $sprite.File -f rawvideo -pix_fmt gray $raw -y
    if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed for $($sprite.File)" }

    $pixels = [IO.File]::ReadAllBytes($raw)
    $canvas = New-Object byte[] (64 * 64)
    $offX = [int]((64 - $sprite.Width) / 2)
    $offY = [int]((64 - $sprite.Height) / 2)

    for ($y = 0; $y -lt $sprite.Height; $y++) {
        for ($x = 0; $x -lt $sprite.Width; $x++) {
            $value = $pixels[$y * $sprite.Width + $x]
            # pokered sprites are four grayscale shades. White is transparent;
            # the other shades map to the species OBJ palette entries 1, 3, 4.
            $color = switch ($value) {
                { $_ -ge 240 } { 0; break }
                { $_ -lt 64 }  { 1; break }
                { $_ -lt 192 } { 3; break }
                default        { 4 }
            }
            $canvas[($offY + $y) * 64 + $offX + $x] = $color
        }
    }

    $lines.Add("const u32 g_battle_$($sprite.Symbol)[BATTLE_WORDS] = {")
    for ($ty = 0; $ty -lt 8; $ty++) {
        for ($tx = 0; $tx -lt 8; $tx++) {
            $words = New-Object System.Collections.Generic.List[string]
            for ($py = 0; $py -lt 8; $py++) {
                [UInt32]$word = 0
                for ($px = 0; $px -lt 8; $px++) {
                    $value = $canvas[($ty * 8 + $py) * 64 + ($tx * 8 + $px)]
                    $word = $word -bor ([UInt32]$value -shl (4 * $px))
                }
                $words.Add(("0x{0:X8}" -f $word))
            }
            $lines.Add("    " + ($words -join ", ") + ",")
        }
    }
    $lines.Add('};')
    $lines.Add('')
}

$lines.Add('static const u32 *front_sprite(PokemonId species) {')
$lines.Add('    switch (species) {')
$lines.Add('    case MON_CHARMANDER: return g_battle_charmander_front;')
$lines.Add('    case MON_SQUIRTLE: return g_battle_squirtle_front;')
 $lines.Add('    case MON_PIDGEY: return g_battle_pidgey_front;')
 $lines.Add('    case MON_RATTATA: return g_battle_rattata_front;')
$lines.Add('    default: return g_battle_bulbasaur_front;')
$lines.Add('    }')
$lines.Add('}')
$lines.Add('')
$lines.Add('static const u32 *back_sprite(PokemonId species) {')
$lines.Add('    switch (species) {')
$lines.Add('    case MON_CHARMANDER: return g_battle_charmander_back;')
$lines.Add('    case MON_SQUIRTLE: return g_battle_squirtle_back;')
$lines.Add('    case MON_PIDGEY: return g_battle_pidgey_back;')
$lines.Add('    case MON_RATTATA: return g_battle_rattata_back;')
$lines.Add('    default: return g_battle_bulbasaur_back;')
$lines.Add('    }')
$lines.Add('}')
$lines.Add('')
$lines.Add('void battle_sprite_load_front(PokemonId species, u32 *dest) {')
$lines.Add('    for (u32 i = 0; i < BATTLE_SPRITE_WORDS; i++) dest[i] = front_sprite(species)[i];')
$lines.Add('}')
$lines.Add('')
$lines.Add('void battle_sprite_load_back(PokemonId species, u32 *dest) {')
$lines.Add('    for (u32 i = 0; i < BATTLE_SPRITE_WORDS; i++) dest[i] = back_sprite(species)[i];')
$lines.Add('}')
$lines.Add('')

Set-Content -LiteralPath $OutputFile -Value $lines -Encoding ascii
