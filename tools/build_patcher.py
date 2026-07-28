#!/usr/bin/env python3
"""
Generate patcher/index.html from the compiled GBA ROM.

The GBA ROM is XOR-encoded against a counter-mode keystream derived from
the canonical Pokemon Red SHA-1 hash. The browser verifies the user's ROM,
derives the same keystream via WebCrypto, and decodes the output locally.
Without a legitimate ROM the embedded bytes are indistinguishable from noise.

Usage:
    python tools/build_patcher.py [pokered_remaster.gba] [patcher/index.html]
"""
from __future__ import annotations

import base64
import hashlib
import json
import struct
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from string import Template

KNOWN_SHA1 = [
    "ea9bcae617fdf159b045185467ae58b2e4a48b9a",  # Pokemon Red (UE) [S][!]
]
CANONICAL = KNOWN_SHA1[0]
VERSION_FILE = Path("patcher/VERSION")
SPLASH_PATH  = Path("patcher/splash.png")

# Reference totals derived from refs/pokered (static — won't change).
REF_MAPS    = 223   # refs/pokered/data/maps/headers/*.asm
REF_POKEMON = 151
REF_MOVES   = 165
REF_MUSIC   = 46    # refs/pokered/audio/music/*.asm
REF_ITEMS   = 97    # li entries in refs/pokered/data/items/names.asm


def _read_version() -> str:
    if VERSION_FILE.exists():
        return VERSION_FILE.read_text().strip()
    return "0.0.0"


def _bump_version(version: str) -> str:
    major, minor, patch = map(int, version.split("."))
    patch += 1
    if patch > 99:
        patch = 0
        minor += 1
    if minor > 9:
        minor = 0
        major += 1
    return f"{major}.{minor}.{patch}"


def _keystream(sha1_hex: str, length: int) -> bytes:
    seed = bytes.fromhex(sha1_hex)
    buf = bytearray()
    n = 0
    while len(buf) < length:
        buf += hashlib.sha256(seed + struct.pack(">I", n)).digest()
        n += 1
    return bytes(buf[:length])


def _xor(data: bytes, key: bytes) -> bytes:
    return bytes(a ^ b for a, b in zip(data, key))


def _count_ported() -> dict:
    """Scan src/ and count how many items in each category have been ported."""
    src = Path("src")

    # Maps: each map_*.c in src/data/ is one ported map.
    maps = len(list((src / "data").glob("map_*.c")))

    # Pokémon: count [N] = { entries in pokemon_base_stats.c, skipping [0].
    pokemon = 0
    pbs = src / "data" / "pokemon_base_stats.c"
    if pbs.exists():
        for line in pbs.read_text(encoding="utf-8").splitlines():
            s = line.strip()
            if s.startswith("[") and s[1:].split("]")[0].isdigit():
                idx = int(s[1:].split("]")[0])
                if idx > 0:
                    pokemon += 1

    # Moves: count [N] = { entries in moves.c, skipping [0].
    moves = 0
    mv = src / "data" / "moves.c"
    if mv.exists():
        for line in mv.read_text(encoding="utf-8").splitlines():
            s = line.strip()
            if s.startswith("[") and s[1:].split("]")[0].isdigit():
                idx = int(s[1:].split("]")[0])
                if idx > 0:
                    moves += 1

    # Music: each *_audio_data.c in src/audio/ is one ported track.
    music = len(list((src / "audio").glob("*_audio_data.c")))

    # Items: count [ITEM_X] = { entries in item.c, skipping ITEM_NONE.
    items = 0
    it = src / "engine" / "item.c"
    if it.exists():
        for line in it.read_text(encoding="utf-8").splitlines():
            s = line.strip()
            if s.startswith("[ITEM_") and "ITEM_NONE" not in s:
                items += 1

    return {
        "maps":    {"ported": maps,    "total": REF_MAPS},
        "pokemon": {"ported": pokemon, "total": REF_POKEMON},
        "moves":   {"ported": moves,   "total": REF_MOVES},
        "music":   {"ported": music,   "total": REF_MUSIC},
        "items":   {"ported": items,   "total": REF_ITEMS},
    }


def _pokemon_with_sprites() -> set[str]:
    """
    Return the set of Pokémon names (lowercase, e.g. 'bulbasaur') that have
    a non-placeholder front sprite in gfx_battle_sprites.c.
    """
    import re
    sprite_file = Path("src/data/gfx_battle_sprites.c")
    names: set[str] = set()
    if sprite_file.exists():
        content = sprite_file.read_text(encoding="utf-8")
        for m in re.finditer(
            r"const u32 g_battle_(\w+)_front\[.*?\] = \{(.*?)\};",
            content, re.DOTALL
        ):
            values = re.findall(r"0x([0-9a-fA-F]+)", m.group(2))
            if any(v != "00000000" for v in values):
                names.add(m.group(1))
    return names


def _pokemon_with_learnsets() -> set[str]:
    """
    Return the set of Pokémon names (lowercase, e.g. 'bulbasaur') that have
    a learnset entry in src/data/learnsets.c.
    Entries are expected as [MON_NAME] = { ... }, skipping MON_NONE.
    """
    learnset_file = Path("src/data/learnsets.c")
    names: set[str] = set()
    if learnset_file.exists():
        for line in learnset_file.read_text(encoding="utf-8").splitlines():
            s = line.strip()
            if s.startswith("[MON_") and "MON_NONE" not in s:
                # [MON_NIDORAN_F] -> nidoran_f
                name = s[5:s.index("]")].lower()
                names.add(name)
    return names


def _pokemon_with_pokedex() -> set[str]:
    """
    Return the set of Pokémon names (lowercase) wired into
    pokedex_species_to_entry() in src/engine/pokedex.c.
    """
    import re
    pokedex_file = Path("src/engine/pokedex.c")
    names: set[str] = set()
    if pokedex_file.exists():
        in_func = False
        for line in pokedex_file.read_text(encoding="utf-8").splitlines():
            if "pokedex_species_to_entry" in line and "{" in line:
                in_func = True
            if in_func:
                m = re.match(r"\s*case MON_(\w+):", line)
                if m:
                    names.add(m.group(1).lower())
                if "}" in line and "case" not in line and "default" not in line:
                    in_func = False
    return names


def _count_learnsets(names: set[str]) -> dict:
    return {"ported": len(names), "total": REF_POKEMON}


def _count_pokedex(names: set[str]) -> dict:
    return {"ported": len(names), "total": REF_POKEMON}


def _count_pokemon_complete(sprite_names: set[str]) -> dict:
    return {"ready": len(sprite_names), "total": REF_POKEMON}


def _count_fully_ported(sprite_names: set[str], learnset_names: set[str], pokedex_names: set[str]) -> dict:
    """Pokémon that have a real sprite, a learnset, AND a Pokédex entry."""
    complete = sprite_names & learnset_names & pokedex_names
    return {"ported": len(complete), "total": REF_POKEMON}


def _calc_overall_pct(counts: dict, fully_ported: dict) -> int:
    """Average the main progress bars, using fully ported Pokémon as the Pokémon metric."""
    bars = [
        counts["maps"]["ported"]     / counts["maps"]["total"],
        counts["music"]["ported"]    / counts["music"]["total"],
        counts["items"]["ported"]    / counts["items"]["total"],
        fully_ported["ported"]       / fully_ported["total"],
    ]
    return max(1, round(sum(bars) / len(bars) * 100))


def _read_splash() -> str:
    """Return a data URI for the splash image, or empty string if not found."""
    if SPLASH_PATH.exists():
        data = SPLASH_PATH.read_bytes()
        b64 = base64.b64encode(data).decode("ascii")
        return f"data:image/png;base64,{b64}"
    return ""


def _git_info() -> tuple[int, str]:
    """Return (commit_count, last_commit_date)."""
    try:
        count = int(subprocess.check_output(
            ["git", "rev-list", "--count", "HEAD"],
            stderr=subprocess.DEVNULL, text=True
        ).strip())
        date = subprocess.check_output(
            ["git", "log", "-1", "--format=%ci"],
            stderr=subprocess.DEVNULL, text=True
        ).strip()[:10]
        return count, date
    except Exception:
        return 0, datetime.now(timezone.utc).strftime("%Y-%m-%d")


def _git_changelog() -> list[dict[str, str]]:
    """Return the commits included in this build, newest first, as GitHub links."""
    try:
        remote = subprocess.check_output(
            ["git", "config", "--get", "remote.origin.url"],
            stderr=subprocess.DEVNULL, text=True
        ).strip()
        if remote.startswith("git@github.com:"):
            repo = remote.removeprefix("git@github.com:")
        else:
            repo = remote.split("github.com/", 1)[1]
        repo = repo.removesuffix(".git").rstrip("/")
        raw = subprocess.check_output(
            ["git", "log", "--date=short", "--pretty=format:%H%x09%h%x09%ad%x09%s"],
            stderr=subprocess.DEVNULL, text=True
        )
        commits = []
        for line in raw.splitlines():
            parts = line.split("\t", 3)
            if len(parts) != 4:
                continue
            full, short, date, subject = parts
            commits.append({
                "hash": full,
                "short": short,
                "date": date,
                "subject": subject,
                "url": f"https://github.com/{repo}/commit/{full}",
            })
        return commits
    except Exception:
        return []


_HTML = Template(r"""<!DOCTYPE html>
<html lang="en" class="gb">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Pokemon Red GBA Remaster - Patcher</title>
<link rel="stylesheet" href="css-pokemon-gameboy.css">
<style>
html,body{margin:0;padding:0;background:#f8f3f8;min-height:100vh}
*{font-family:'Pokemon GB',monospace}
body{display:flex;justify-content:center;padding:32px 16px}
#screen{max-width:860px;width:100%;display:flex;flex-direction:column;gap:0}
h1{font-size:1.4rem;text-align:center;line-height:1.8;margin:0}
.sub{font-size:.75rem;text-align:center;display:block;margin-top:.25em;color:#e3000b}
#drop{cursor:pointer;user-select:none;display:flex;flex-direction:column}
#drop-in{display:flex;align-items:center;justify-content:center;text-align:center;padding:1.5em;font-size:.75rem}
#dl-wrap{display:none}
#dl-wrap ul{width:100%}
#dl-wrap li{margin:.5em 0}
#sf{display:none}
.desc-text{font-size:.7rem;line-height:1.7}
.framed.neutral{font-size:.75rem}
#prog .phase-hd{font-size:.65rem;margin-bottom:.8em}
#prog .pct-hd{font-size:.55rem;margin-top:.5em;opacity:.65;margin-bottom:.6em}
.stat-grid{display:flex;flex-direction:column;gap:.7em;margin-top:.6em}
.stat-row{display:flex;flex-direction:column;gap:.3em}
.stat-meta{display:flex;justify-content:space-between;align-items:baseline}
.stat-lbl{font-size:.65rem}
.stat-num{font-size:.6rem;opacity:.7}
.stat-bar-wrap{width:100%;height:5px;background:rgba(0,0,0,.12)}
.stat-bar-fill{height:100%;background:#e3000b}
#pkmn-prog .pkmn-hd{font-size:.75rem;margin-bottom:.8em}
#changelog .pkmn-hd{font-size:.75rem;margin-bottom:.8em}
.changelog-list{display:flex;flex-direction:column;gap:.45em}
.changelog-item{display:flex;justify-content:space-between;gap:1em;align-items:baseline;font-size:.65rem}
.changelog-item a{color:inherit;text-decoration:none;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.changelog-item a:hover{text-decoration:underline}
#splash{text-align:center;padding:1em 0}
#splash img{max-width:100%;height:auto;image-rendering:pixelated;image-rendering:crisp-edges}
.footer-note{font-size:.6rem}

html.gb h1 span{color:#e3000b}
html.gb #drop.over #drop-in{background:#48a058!important;color:#f8f3f8!important}

html.gba body{background:#0a0a0a}
html.gba #screen{color:#d8d8d8}
html.gba .framed.primary{background:#1a4d2e!important;color:#d0f0d8!important}
html.gba .framed.neutral{background:#1c1c1c!important;color:#d8d8d8!important}
html.gba .framed.danger{background:#3d0f0f!important;color:#f8c0c0!important}
html.gba #drop.over #drop-in{background:#1a3a22!important}
html.gba a.button{color:#d0f0d8;background:#1a4d2e;display:block;padding:14px 16px;border-radius:6px;text-align:center;text-decoration:none}
html.gba a.button:hover{background:#113320}
html.gba a.button:before{display:none}
html.gba .stat-bar-wrap{background:rgba(255,255,255,.1)}
</style>
</head>
<body>
<div id="screen">
<div class="framed neutral">
<h1>POKEMON RED<br><span>GBA REMASTER</span></h1>
<span class="sub">OWNERSHIP VERIFICATION PATCHER v$VERSION</span>
</div>
<div class="framed neutral no-hd">
<span class="desc-text">NATIVE GBA PORT OF POKEMON RED &#8212; REBUILT FROM SOURCE IN C USING DEVKITARM. RUNS ON REAL GBA HARDWARE AND EMULATORS. NOT A ROM HACK.</span>
</div>
<div class="framed neutral no-hd">
SUPPLY YOUR POKEMON RED GAME BOY ROM TO VERIFY OWNERSHIP AND RECEIVE THE GBA BUILD.
YOUR FILE IS NEVER UPLOADED.
</div>
<div id="drop" onclick="document.getElementById('f').click()">
<div class="framed neutral no-hd" id="drop-in">
&#127918; CLICK OR DROP YOUR POKEMON RED .GB ROM HERE
<input id="f" type="file" accept="*/*" style="display:none">
</div>
</div>
<div class="framed neutral no-hd" id="sf"><div id="st"></div></div>
<div id="dl-wrap">
<ul class="buttons">
<li><a id="dl" class="button">&#11015; DOWNLOAD GBA ROM</a></li>
</ul>
</div>
<div class="framed neutral no-hd footer-note">
REQUIRES POKEMON RED (UE) [S][!] &#183; ROM NEVER LEAVES YOUR DEVICE
</div>
$SPLASH_HTML
<div class="framed neutral no-hd" id="prog">
<div id="prog-hd" class="phase-hd"></div>
<div class="progress-bar-container">
<div id="pbar" class="progress-bar"></div>
</div>
<div id="prog-pct" class="pct-hd"></div>
<div id="stat-grid" class="stat-grid"></div>
</div>
<div class="framed neutral no-hd" id="pkmn-prog">
<div class="pkmn-hd">POKEMON COMPLETENESS</div>
<div id="pkmn-grid" class="stat-grid"></div>
</div>
<div class="framed neutral no-hd" id="changelog">
<div class="pkmn-hd">CHANGELOG</div>
<div id="changelog-list" class="changelog-list"></div>
</div>
</div>
<script>
const K=$HASHES;
const E="$BLOB";
const _VER="$VERSION";
const _COMMITS=$COMMIT_COUNT;
const _BUILD="$BUILD_DATE";
const _PCT=$OVERALL_PCT;
const _COUNTS=$COUNTS_JSON;
const _PKMN_COMPLETE=$PKMN_COMPLETE_JSON;
const _LEARNSETS=$LEARNSETS_JSON;
const _POKEDEX=$POKEDEX_JSON;
const _FULLY_PORTED=$FULLY_PORTED_JSON;
const _CHANGELOG=$CHANGELOG_JSON;
const _LABELS={maps:"Maps",pokemon:"Pokémon",moves:"Moves",music:"Music",items:"Items"};
(function(){
  document.getElementById("pbar").className="progress-bar p"+_PCT;
  var shortDate=_BUILD.slice(5).replace("-","/");
  document.getElementById("prog-hd").textContent="PORT PROGRESS  v"+_VER+"  #"+_COMMITS+"  "+shortDate;
  document.getElementById("prog-pct").textContent=_PCT+"% TO v1.0.0";
  var grid=document.getElementById("stat-grid");
  var progressRows=[
    {label:"Maps",    ported:_COUNTS.maps.ported,    total:_COUNTS.maps.total},
    {label:"Pokémon", ported:_FULLY_PORTED.ported, total:_FULLY_PORTED.total},
    {label:"Music",   ported:_COUNTS.music.ported,   total:_COUNTS.music.total},
    {label:"Items",   ported:_COUNTS.items.ported,   total:_COUNTS.items.total},
  ];
  for(var i=0;i<progressRows.length;i++){
    var d=progressRows[i];
    var pct=Math.round(d.ported/d.total*100);
    var row=document.createElement("div");row.className="stat-row";
    var meta=document.createElement("div");meta.className="stat-meta";
    var lbl=document.createElement("span");lbl.className="stat-lbl";lbl.textContent=d.label;
    var num=document.createElement("span");num.className="stat-num";num.textContent=d.ported+"/"+d.total;
    meta.appendChild(lbl);meta.appendChild(num);
    var wrap=document.createElement("div");wrap.className="stat-bar-wrap";
    var fill=document.createElement("div");fill.className="stat-bar-fill";fill.style.width=pct+"%";
    wrap.appendChild(fill);
    row.appendChild(meta);row.appendChild(wrap);
    grid.appendChild(row);
  }
  var pkRows=[
    {label:"Sprites",       ported:_PKMN_COMPLETE.ready,  total:_PKMN_COMPLETE.total},
    {label:"Learnsets",     ported:_LEARNSETS.ported,      total:_LEARNSETS.total},
    {label:"Pokédex",  ported:_POKEDEX.ported,        total:_POKEDEX.total},
    {label:"Moves",         ported:_COUNTS.moves.ported,    total:_COUNTS.moves.total},
  ];
  var pkGrid=document.getElementById("pkmn-grid");
  for(var i=0;i<pkRows.length;i++){
    var r=pkRows[i];
    var pct=Math.round(r.ported/r.total*100);
    var row=document.createElement("div");row.className="stat-row";
    var meta=document.createElement("div");meta.className="stat-meta";
    var lbl=document.createElement("span");lbl.className="stat-lbl";lbl.textContent=r.label;
    var num=document.createElement("span");num.className="stat-num";num.textContent=r.ported+"/"+r.total;
    meta.appendChild(lbl);meta.appendChild(num);
    var wrap=document.createElement("div");wrap.className="stat-bar-wrap";
    var fill=document.createElement("div");fill.className="stat-bar-fill";fill.style.width=Math.max(pct,pct>0?1:0)+"%";
    wrap.appendChild(fill);
    row.appendChild(meta);row.appendChild(wrap);
    pkGrid.appendChild(row);
  }
  var changelog=document.getElementById("changelog-list");
  for(var i=0;i<_CHANGELOG.length;i++){
    var c=_CHANGELOG[i], item=document.createElement("div");item.className="changelog-item";
    var link=document.createElement("a");link.href=c.url;link.target="_blank";link.rel="noopener";
    link.textContent=c.subject;link.title=c.hash;
    var meta=document.createElement("span");meta.className="stat-num";meta.textContent=c.short+"  "+c.date;
    item.appendChild(link);item.appendChild(meta);changelog.appendChild(item);
  }
})();
async function sha1(buf){const h=await crypto.subtle.digest("SHA-1",buf);return[...new Uint8Array(h)].map(b=>b.toString(16).padStart(2,"0")).join("")}
async function keystream(hex,n){const s=Uint8Array.from(hex.match(/../g),x=>parseInt(x,16));const cnt=Math.ceil(n/32);const inputs=Array.from({length:cnt},(_,i)=>{const q=new Uint8Array(24);q.set(s);new DataView(q.buffer).setUint32(20,i,false);return q});const blocks=await Promise.all(inputs.map(q=>crypto.subtle.digest("SHA-256",q)));const o=new Uint8Array(n);let p=0;for(const b of blocks){const a=new Uint8Array(b);const c=Math.min(32,n-p);o.set(a.subarray(0,c),p);p+=c}return o}
async function run(file){const sf=document.getElementById("sf"),st=document.getElementById("st"),dlw=document.getElementById("dl-wrap"),dl=document.getElementById("dl");sf.className="framed neutral no-hd";sf.style.display="block";dlw.style.display="none";st.textContent="VERIFYING...";try{const buf=await file.arrayBuffer();const hash=await sha1(buf);if(!K.includes(hash)){sf.className="framed danger no-hd";st.textContent="ROM NOT RECOGNISED. REQUIRES POKEMON RED (UE) [S][!]";return}st.textContent="DECODING...";const enc=Uint8Array.from(atob(E),c=>c.charCodeAt(0));const key=await keystream(K[0],enc.length);const out=new Uint8Array(enc.length);for(let i=0;i<enc.length;i++)out[i]=enc[i]^key[i];const url=URL.createObjectURL(new Blob([out],{type:"application/octet-stream"}));dl.href=url;dl.download="pokered_remaster_v$VERSION.gba";document.documentElement.className="gba";sf.className="framed primary no-hd";st.textContent="ROM VERIFIED. YOUR DOWNLOAD IS READY.";dlw.style.display="block"}catch(e){sf.className="framed danger no-hd";st.textContent="ERROR: "+e.message}}
const d=document.getElementById("drop");
d.ondragover=e=>{e.preventDefault();d.classList.add("over")};
d.ondragleave=()=>d.classList.remove("over");
d.ondrop=e=>{e.preventDefault();d.classList.remove("over");const f=e.dataTransfer.files[0];if(f)run(f)};
document.getElementById("f").onchange=e=>{if(e.target.files[0])run(e.target.files[0])};
</script>
</body>
</html>""")


def build(gba_path: Path, out_path: Path) -> None:
    version = _read_version()
    print(f"  Version  v{version}")
    print(f"  Reading  {gba_path} ...")
    rom = gba_path.read_bytes()
    print(f"  Encoding {len(rom):,} bytes ...")
    encoded = _xor(rom, _keystream(CANONICAL, len(rom)))
    blob = base64.b64encode(encoded).decode("ascii")
    hashes_js = "[" + ",".join(f'"{h}"' for h in KNOWN_SHA1) + "]"

    counts         = _count_ported()
    sprite_names   = _pokemon_with_sprites()
    learnset_names = _pokemon_with_learnsets()
    pokedex_names  = _pokemon_with_pokedex()
    pkmn_complete  = _count_pokemon_complete(sprite_names)
    learnsets      = _count_learnsets(learnset_names)
    pokedex        = _count_pokedex(pokedex_names)
    fully_ported   = _count_fully_ported(sprite_names, learnset_names, pokedex_names)
    overall_pct    = _calc_overall_pct(counts, fully_ported)
    print(f"  Progress {overall_pct}%  maps={counts['maps']['ported']}/{counts['maps']['total']}"
          f"  pokemon={counts['pokemon']['ported']}/{counts['pokemon']['total']}"
          f"  moves={counts['moves']['ported']}/{counts['moves']['total']}"
          f"  music={counts['music']['ported']}/{counts['music']['total']}"
          f"  items={counts['items']['ported']}/{counts['items']['total']}"
          f"  sprites={pkmn_complete['ready']}/{pkmn_complete['total']}"
          f"  learnsets={learnsets['ported']}/{learnsets['total']}"
          f"  pokedex={pokedex['ported']}/{pokedex['total']}"
          f"  fully-ported={fully_ported['ported']}/{fully_ported['total']}")

    splash_data = _read_splash()
    if splash_data:
        splash_html = (
            '<div class="framed neutral no-hd" id="splash">'
            f'<img src="{splash_data}" alt="Pokemon Red GBA Remaster"></div>'
        )
        print(f"  Splash   {SPLASH_PATH} embedded")
    else:
        splash_html = ""
        print(f"  Splash   (none — place image at {SPLASH_PATH} to embed)")

    commit_count, build_date = _git_info()
    changelog           = _git_changelog()
    counts_json        = json.dumps(counts, separators=(",", ":"))
    pkmn_complete_json = json.dumps(pkmn_complete, separators=(",", ":"))
    learnsets_json     = json.dumps(learnsets, separators=(",", ":"))
    pokedex_json       = json.dumps(pokedex, separators=(",", ":"))
    fully_ported_json  = json.dumps(fully_ported, separators=(",", ":"))
    changelog_json     = json.dumps(changelog, ensure_ascii=False, separators=(",", ":"))

    html = _HTML.substitute(
        HASHES=hashes_js,
        BLOB=blob,
        VERSION=version,
        SPLASH_HTML=splash_html,
        COMMIT_COUNT=commit_count,
        BUILD_DATE=build_date,
        OVERALL_PCT=overall_pct,
        COUNTS_JSON=counts_json,
        PKMN_COMPLETE_JSON=pkmn_complete_json,
        LEARNSETS_JSON=learnsets_json,
        POKEDEX_JSON=pokedex_json,
        FULLY_PORTED_JSON=fully_ported_json,
        CHANGELOG_JSON=changelog_json,
    )
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(html, encoding="utf-8")
    VERSION_FILE.write_text(_bump_version(version) + "\n")
    kb = out_path.stat().st_size // 1024
    print(f"  Built    {out_path}  ({kb} KB)  [next: v{_bump_version(version)}]")


if __name__ == "__main__":
    gba = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("pokered_remaster.gba")
    out = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("patcher/index.html")
    if not gba.exists():
        sys.exit(f"error: {gba} not found -- run make first")
    build(gba, out)
