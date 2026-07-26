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
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Pokemon Red GBA Remaster - Patcher</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:#0d0d1a;color:#e8e8e8;font-family:system-ui,sans-serif;min-height:100vh;display:flex;align-items:center;justify-content:center;padding:24px}
.card{background:#16213e;border-radius:12px;padding:40px 36px;max-width:480px;width:100%;box-shadow:0 8px 32px rgba(0,0,0,.5)}
h1{font-size:1.6rem;font-weight:700;text-align:center;margin-bottom:6px}
h1 span{color:#e3000b}
.sub{text-align:center;color:#9aa5b4;font-size:.85rem;margin-bottom:24px}
.intro{color:#9aa5b4;font-size:.88rem;margin-bottom:20px;line-height:1.65}
.drop{border:2px dashed #2d4a7a;border-radius:8px;padding:36px 20px;text-align:center;cursor:pointer;transition:.15s;background:#0f1829;user-select:none}
.drop:hover,.drop.over{border-color:#e3000b;background:#150808}
.drop-icon{font-size:2.2rem;margin-bottom:10px}
.drop-label{color:#9aa5b4;font-size:.9rem}
.drop-label strong{color:#e8e8e8}
#status{margin-top:20px;min-height:22px;text-align:center;font-size:.88rem;font-weight:500;transition:.2s}
.ok{color:#00c851}
.err{color:#ff4444}
.info{color:#9aa5b4}
#dl{display:none;margin-top:16px;padding:14px 0;background:#e3000b;color:#fff;border-radius:8px;font-size:1rem;font-weight:600;text-align:center;text-decoration:none;width:100%}
#dl:hover{background:#b8000a}
.footer{margin-top:24px;color:#3a4a5a;font-size:.76rem;text-align:center;line-height:1.6}
</style>
</head>
<body>
<div class="card">
<h1>Pokemon Red<br><span>GBA Remaster</span></h1>
<p class="sub">ownership verification patcher</p>
<p class="intro">Supply your original <strong style="color:#e8e8e8">Pokemon Red</strong> Game Boy ROM to verify ownership and receive the GBA build. Your file is never uploaded &mdash; everything runs locally in your browser.</p>
<div class="drop" id="drop" onclick="document.getElementById('f').click()">
<div class="drop-icon">&#127918;</div>
<div class="drop-label"><strong>Click or drop</strong> your Pokemon Red .gb ROM here</div>
<input id="f" type="file" accept=".gb,.bin" style="display:none">
</div>
<div id="status"></div>
<a id="dl">&#11015; Download pokered_remaster.gba</a>
<p class="footer">Compatible with Pokemon Red (UE) [S][!]<br>Your ROM is never uploaded or shared.</p>
</div>
<script>
const K=$HASHES;
const E="$BLOB";
async function sha1(buf){const h=await crypto.subtle.digest("SHA-1",buf);return[...new Uint8Array(h)].map(b=>b.toString(16).padStart(2,"0")).join("")}
async function keystream(hex,n){const s=Uint8Array.from(hex.match(/../g),x=>parseInt(x,16));const cnt=Math.ceil(n/32);const inputs=Array.from({length:cnt},(_,i)=>{const q=new Uint8Array(24);q.set(s);new DataView(q.buffer).setUint32(20,i,false);return q});const blocks=await Promise.all(inputs.map(q=>crypto.subtle.digest("SHA-256",q)));const o=new Uint8Array(n);let p=0;for(const b of blocks){const a=new Uint8Array(b);const c=Math.min(32,n-p);o.set(a.subarray(0,c),p);p+=c}return o}
async function run(file){const st=document.getElementById("status"),dl=document.getElementById("dl");st.className="info";st.textContent="Verifying…";dl.style.display="none";try{const buf=await file.arrayBuffer();const hash=await sha1(buf);if(!K.includes(hash)){st.className="err";st.textContent="✗ ROM not recognised — requires Pokemon Red (UE) [S][!]";return}st.textContent="Decoding…";const enc=Uint8Array.from(atob(E),c=>c.charCodeAt(0));const key=await keystream(K[0],enc.length);const out=new Uint8Array(enc.length);for(let i=0;i<enc.length;i++)out[i]=enc[i]^key[i];const url=URL.createObjectURL(new Blob([out],{type:"application/octet-stream"}));dl.href=url;dl.download="pokered_remaster.gba";dl.style.display="block";st.className="ok";st.textContent="✓ ROM verified. Your download is ready."}catch(e){st.className="err";st.textContent="✗ "+e.message}}
const d=document.getElementById("drop");
d.ondragover=e=>{e.preventDefault();d.classList.add("over")};
d.ondragleave=()=>d.classList.remove("over");
d.ondrop=e=>{e.preventDefault();d.classList.remove("over");const f=e.dataTransfer.files[0];if(f)run(f)};
document.getElementById("f").onchange=e=>{if(e.target.files[0])run(e.target.files[0])};
</script>
</body>
</html>""")


def build(gba_path: Path, out_path: Path) -> None:
    print(f"  Reading  {gba_path} ...")
    rom = gba_path.read_bytes()
    print(f"  Encoding {len(rom):,} bytes ...")
    encoded = _xor(rom, _keystream(CANONICAL, len(rom)))
    blob = base64.b64encode(encoded).decode("ascii")
    hashes_js = "[" + ",".join(f'"{h}"' for h in KNOWN_SHA1) + "]"
    html = _HTML.substitute(HASHES=hashes_js, BLOB=blob)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(html, encoding="utf-8")
    kb = out_path.stat().st_size // 1024
    print(f"  Built    {out_path}  ({kb} KB)")


if __name__ == "__main__":
    gba = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("pokered_remaster.gba")
    out = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("patcher/index.html")
    if not gba.exists():
        sys.exit(f"error: {gba} not found -- run make first")
    build(gba, out)
