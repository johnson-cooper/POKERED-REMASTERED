"""Extract pokered map object declarations into a machine-readable manifest.

This is intentionally a source-audit tool, not a second map format.  The
runtime remains data-driven C, while this manifest makes it possible to check
which reference warps, background events, persons, item balls, and trainer
objects have been migrated before a map is considered complete.
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OBJECTS = ROOT / "refs" / "pokered" / "data" / "maps" / "objects"

MAP_HEADER_RE = re.compile(r"^\s*map_header\s+(\w+),\s*(\w+),\s*(\w+)", re.M)
WARP_RE = re.compile(
    r"^\s*warp_event\s+(\d+),\s*(\d+),\s*(\w+),\s*(\d+)", re.M
)
BG_RE = re.compile(r"^\s*bg_event\s+(\d+),\s*(\d+),\s*(\w+)", re.M)
OBJECT_RE = re.compile(r"^\s*object_event\s+(.+)$", re.M)


def split_args(value: str) -> list[str]:
    return [part.strip() for part in value.split(",")]


def parse_object(line: str) -> dict[str, object]:
    args = split_args(line)
    result: dict[str, object] = {"raw": line.strip()}
    if len(args) >= 6:
        result.update(
            x=int(args[0]),
            y=int(args[1]),
            sprite=args[2],
            movement=args[3],
            facing_or_route=args[4],
            text=args[5],
        )
    if len(args) >= 7:
        result["extra"] = args[6:]
        if args[2] == "SPRITE_POKE_BALL":
            result["kind"] = "item_ball"
            result["item"] = args[6]
        elif args[6].startswith("OPP_"):
            result["kind"] = "trainer"
            result["trainer"] = args[6]
            if len(args) >= 8:
                result["trainer_party"] = int(args[7])
        else:
            result["kind"] = "object"
    else:
        result["kind"] = "person"
    return result


def parse_file(path: Path) -> dict[str, object]:
    text = path.read_text(encoding="utf-8", errors="replace")
    header = MAP_HEADER_RE.search(text)
    return {
        "source": str(path.relative_to(ROOT)).replace("\\", "/"),
        "map_symbol": header.group(1) if header else path.stem,
        "map_id": header.group(2) if header else None,
        "tileset": header.group(3) if header else None,
        "warps": [
            {"x": int(x), "y": int(y), "destination": dest, "destination_warp": int(warp)}
            for x, y, dest, warp in WARP_RE.findall(text)
        ],
        "background_events": [
            {"x": int(x), "y": int(y), "text": text_id}
            for x, y, text_id in BG_RE.findall(text)
        ],
        "objects": [parse_object(line) for line in OBJECT_RE.findall(text)],
    }


def build_manifest() -> dict[str, object]:
    maps = [parse_file(path) for path in sorted(OBJECTS.glob("*.asm"))]
    counts = {
        "maps": len(maps),
        "warps": sum(len(m["warps"]) for m in maps),
        "background_events": sum(len(m["background_events"]) for m in maps),
        "objects": sum(len(m["objects"]) for m in maps),
        "item_balls": sum(
            1 for m in maps for obj in m["objects"] if obj.get("kind") == "item_ball"
        ),
        "trainers": sum(
            1 for m in maps for obj in m["objects"] if obj.get("kind") == "trainer"
        ),
    }
    return {"source": "refs/pokered", "counts": counts, "maps": maps}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", type=Path, help="write JSON instead of stdout")
    args = parser.parse_args()
    manifest = build_manifest()
    encoded = json.dumps(manifest, indent=2, sort_keys=False) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8")
        print(f"wrote {args.output} ({manifest['counts']['maps']} maps)")
    else:
        print(encoded, end="")


if __name__ == "__main__":
    main()
