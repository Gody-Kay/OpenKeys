#!/usr/bin/env python3
"""Validate content/presets/*.json and docs/example.performance.json against the schema.
Checks sfz refs exist only when content/sounds/ is present (CI has no sounds).
Exit codes: 0 ok, 1 failure, 77 skipped (jsonschema not installed)."""
import json, pathlib, sys

ROOT = pathlib.Path(__file__).resolve().parent.parent

def main() -> int:
    try:
        import jsonschema
    except ImportError:
        print("SKIP: jsonschema not installed (pip install jsonschema)")
        return 77
    schema = json.loads((ROOT / "docs" / "performance.schema.json").read_text(encoding="utf-8"))
    presets = sorted((ROOT / "content" / "presets").glob("*.json"))
    files = presets + [ROOT / "docs" / "example.performance.json"]
    sounds = ROOT / "content" / "sounds"
    bad = 0
    for f in files:
        try:
            d = json.loads(f.read_text(encoding="utf-8"))
            jsonschema.validate(d, schema)
        except Exception as e:
            print(f"FAIL {f.name}: {str(e).splitlines()[0]}")
            bad += 1
            continue
        if f in presets and sounds.exists():
            for p in d["parts"]:
                ins = p["instrument"]
                if ins["type"] == "sfz" and not (sounds / ins["ref"]).exists():
                    print(f"FAIL {f.name}: missing sfz {ins['ref']}")
                    bad += 1
        print(f"ok   {f.name}")
    if not sounds.exists():
        print("note: content/sounds/ absent, sfz refs not checked")
    return 1 if bad else 0

if __name__ == "__main__":
    sys.exit(main())
