#!/usr/bin/env python3
"""Phase 0 checks: repo layout, schema is valid JSON, example Performance fits it.
Stdlib only (no jsonschema), so it runs on any CI image. Usage: test_repo.py <repo_root>
"""
import json
import sys
from pathlib import Path

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")
errors = []

for rel in ["SESSION_RULES.md", "PLAN.md", "FEATURES.md", "PROGRESS.md", "DECISIONS.md",
            "CMakeLists.txt", "content/LICENSES.md", "docs/performance.schema.json",
            "docs/example.performance.json", "tools/fetch_sounds.py",
            "content/sources.json", ".github/workflows/build.yml"]:
    if not (root / rel).is_file():
        errors.append(f"missing file: {rel}")

for rel in ["src/common", "src/engine", "src/ui", "src/app", "tools", "content", "ci",
            "installers", "docs", "tests/integration"]:
    if not (root / rel).is_dir():
        errors.append(f"missing dir: {rel}")

try:
    schema = json.loads((root / "docs/performance.schema.json").read_text())
    perf = json.loads((root / "docs/example.performance.json").read_text())
except Exception as e:  # noqa: BLE001
    errors.append(f"bad JSON: {e}")
    schema = perf = None

if schema and perf:
    for key in schema.get("required", []):
        if key not in perf:
            errors.append(f"example missing required key: {key}")
    parts = perf.get("parts", [])
    if not 1 <= len(parts) <= 16:
        errors.append("parts must be 1..16")
    if len(perf.get("scenes", [])) > 8:
        errors.append("scenes must be <= 8")
    ids = {p.get("id") for p in parts}
    for s in perf.get("scenes", []):
        for pid in s.get("parts", {}):
            if pid not in ids:
                errors.append(f"scene {s.get('name')} refers to unknown part {pid}")
    for p in parts:
        z = p.get("zone", {})
        if not (0 <= z.get("keyLow", 0) <= z.get("keyHigh", 127) <= 127):
            errors.append(f"part {p.get('id')}: bad key range")

lic = root / "content/LICENSES.md"
if lic.is_file() and "JUCE" not in lic.read_text():
    errors.append("LICENSES.md must list JUCE")

if errors:
    print("FAIL")
    for e in errors:
        print(" -", e)
    sys.exit(1)
print("PASS repo layout + schema checks")
