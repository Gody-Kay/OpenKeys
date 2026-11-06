#!/usr/bin/env python3
"""Fail if a library in content/sources.json has no line in content/LICENSES.md
or uses a licence that is not on the allow-list."""
import json, pathlib, sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
ALLOWED = {"CC0-1.0", "CC-BY-4.0", "CC-BY-3.0", "PUBLIC-DOMAIN", "MIT", "BSD-2-Clause", "BSD-3-Clause", "Apache-2.0"}

def main() -> int:
    sources = json.loads((ROOT / "content" / "sources.json").read_text(encoding="utf-8"))
    licenses = (ROOT / "content" / "LICENSES.md").read_text(encoding="utf-8")
    bad = []
    for lib in sources.get("libraries", []):
        name = lib.get("name", "?")
        if lib.get("license") not in ALLOWED:
            bad.append(f"{name}: licence {lib.get('license')!r} not allowed")
        if name not in licenses:
            bad.append(f"{name}: missing from content/LICENSES.md")
    if bad:
        print("LICENCE CHECK FAILED")
        print("\n".join(" - " + b for b in bad))
        return 1
    print(f"licence check ok ({len(sources.get('libraries', []))} libraries)")
    return 0

if __name__ == "__main__":
    sys.exit(main())
