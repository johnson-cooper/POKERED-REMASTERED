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
import struct
import sys
from pathlib import Path
from string import Template

KNOWN_SHA1 = [
    "ea9bcae617fdf159b045185467ae58b2e4a48b9a",  # Pokemon Red (UE) [S][!]
]
CANONICAL = KNOWN_SHA1[0]
VERSION_FILE = Path("patcher/VERSION")


def _read_version() -> str:
    if VERSION_FILE.exists():
        return VERSION_FILE.read_text().strip()
    return "0.0.0"


def _bump_version(version: str) -> str:
    parts = version.split(".")
    parts[-1] = str(int(parts[-1]) + 1)
    return ".".join(parts)


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


_HTML = Template(r"""<!DOCTYPE html>
<html lang="en" class="gb">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Pokemon Red GBA Remaster - Patcher</title>
<link rel="stylesheet" href="css-pokemon-gameboy.css">
<style>
html,body{margin:0;padding:0;min-height:100vh;transition:background .5s}
*{font-family:'Pokemon GB',monospace}
body{display:flex;align-items:stretch;justify-content:center;padding:20px}
#screen{max-width:680px;width:100%;min-height:calc(100vh - 40px);display:flex;flex-direction:column;transition:background .5s,box-shadow .5s,border-radius .5s,padding .5s}
h1{font-size:1rem;text-align:center;line-height:1.8;margin:0}
.sub{font-size:.5rem;text-align:center;display:block;margin-top:.25em}
#drop{cursor:pointer;user-select:none;flex:1;display:flex;flex-direction:column}
#drop-in{flex:1;display:flex;align-items:center;justify-content:center;text-align:center}
#dl-wrap{display:none}
#dl-wrap ul{width:100%}
#dl-wrap li{margin:.5em 0}
#sf{display:none}

html.gb body{background:#f8f3f8}
html.gb #screen{background:#f8f3f8;padding:20px;border:4px solid #181010;box-shadow:0 0 0 4px #e3000b,0 0 0 10px #181010}
html.gb h1 span{color:#e3000b}
html.gb .sub{color:#e3000b}
html.gb #drop.over #drop-in{background:#48a058!important;color:#f8f3f8!important}

html.gba body{background:#080808}
html.gba #screen{background:#111111;padding:32px;border-radius:12px;box-shadow:0 8px 40px rgba(0,0,0,.9)}
html.gba h1 span{color:#e3000b}
html.gba .sub{color:#e3000b}
html.gba .framed.primary{background:#1a4d2e!important;color:#d0f0d8!important}
html.gba .framed.neutral{background:#1c1c1c!important;color:#d8d8d8!important}
html.gba .framed.danger{background:#3d0f0f!important;color:#f8c0c0!important}
html.gba #drop.over #drop-in{background:#1a3a22!important}
html.gba a.button{color:#d0f0d8;background:#1a4d2e;display:block;padding:14px 16px;border-radius:6px;text-align:center;text-decoration:none}
html.gba a.button:hover{background:#113320}
html.gba a.button:before{display:none}
</style>
</head>
<body>
<div id="screen">
<div class="framed neutral">
<h1>POKEMON RED<br><span>GBA REMASTER</span></h1>
<span class="sub">OWNERSHIP VERIFICATION PATCHER v$VERSION</span>
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
<div class="framed neutral no-hd" style="font-size:.45rem">
REQUIRES POKEMON RED (UE) [S][!] &#183; ROM NEVER LEAVES YOUR DEVICE
</div>
</div>
<script>
const K=$HASHES;
const E="$BLOB";
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
    html = _HTML.substitute(HASHES=hashes_js, BLOB=blob, VERSION=version)
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
