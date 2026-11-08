#!/usr/bin/env bash
# Finish the VSCO 2 CE import after the download completes. Safe to re-run. Run: bash tools/finish_vsco.sh
cd "$(dirname "$0")/.." || exit 1
d=content/sounds/vsco
n=$(find "$d" -name '*.wav' 2>/dev/null | wc -l); echo "wav files: $n"
[ "$n" -ge 2500 ] || { echo "NOT COMPLETE (need 2500+ wav files)."; exit 1; }
if [ -d "$d/.git" ]; then
  [ -z "$(git -C "$d" status --porcelain | head -n 1)" ] || { echo "Checkout incomplete."; exit 1; }
  git -C "$d" rev-parse --short HEAD > "$d/.source-commit"
  rm -rf "$d/.git"
fi
C=$(cat "$d/.source-commit" 2>/dev/null || echo unknown)
python3 - "$C" <<'PY'
import sys, pathlib
p = pathlib.Path("content/LICENSES.md"); s = p.read_text(encoding="utf-8")
if "VSCO 2 Community Edition" not in s:
    row = f"| VSCO 2 Community Edition (SFZ branch) | CC0-1.0 | https://github.com/sgossner/VSCO-2-CE | Credit requested, not required: Versilian Studios LLC / Sam Gossner | Commit {sys.argv[1]}. Strings, brass, woodwinds, percussion, keys. Readme asks not to resell as-is; the CC0 LICENSE governs. |"
    lines = s.split("\n"); i = max(k for k, l in enumerate(lines) if l.startswith("| Salamander"))
    lines.insert(i + 1, row); p.write_text("\n".join(lines), encoding="utf-8")
PY
mkdir -p content/presets
cat > content/presets/strings-ensemble.json <<'JSON'
{
  "schemaVersion": 1, "name": "Strings Ensemble", "category": "Strings", "tempo": 72,
  "master": { "transpose": 0, "octaveShift": 0, "tuneHz": 440, "mono": false },
  "parts": [
    { "id": 1, "name": "Cellos", "enabled": true, "color": "#FFB74D",
      "instrument": { "type": "sfz", "ref": "vsco/CelloEnsSusVib.sfz" },
      "keyRange": { "low": 0, "high": 59 }, "velocityRange": { "low": 1, "high": 127 },
      "velocityCurve": "normal", "volume": 0.7, "pan": -0.2, "sends": { "reverb": 0.3 } },
    { "id": 2, "name": "Violas", "enabled": true, "color": "#F06292",
      "instrument": { "type": "sfz", "ref": "vsco/ViolaEnsSusVib.sfz" },
      "keyRange": { "low": 48, "high": 79 }, "velocityRange": { "low": 1, "high": 127 },
      "velocityCurve": "normal", "volume": 0.7, "pan": 0.0, "sends": { "reverb": 0.3 } },
    { "id": 3, "name": "Violins", "enabled": true, "color": "#4DB6AC",
      "instrument": { "type": "sfz", "ref": "vsco/ViolinEnsSusVib.sfz" },
      "keyRange": { "low": 60, "high": 127 }, "velocityRange": { "low": 1, "high": 127 },
      "velocityCurve": "normal", "volume": 0.7, "pan": 0.2, "sends": { "reverb": 0.3 } }
  ],
  "scenes": [ { "name": "Main", "parts": { "1": { "enabled": true }, "2": { "enabled": true }, "3": { "enabled": true } } } ]
}
JSON
python3 tools/check_presets.py || { echo "PRESET CHECK FAILED, nothing committed"; exit 1; }
python3 tools/check_licenses.py
D=$(date '+%Y-%m-%d'); T=$(date '+%Y-%m-%d %H:%M')
grep -q "DC4" DECISIONS.md || echo "- DC4 (C, assumed) $D: strings-ensemble layers Cello/Viola/Violin sections by key range (0-59, 48-79, 60-127). NOT checked by ear." >> DECISIONS.md
echo "- [C] $T VSCO 2 CE (CC0, commit $C) imported to content/sounds/vsco (gitignored, .git removed). Preset strings-ensemble.json added. Sound NOT verified by ear; sfizz not integrated yet." >> PROGRESS.md
git checkout session-c -q && git add -A && git commit -qm "content: VSCO 2 CE licence + strings preset" && git push -q && git checkout main -q && git merge -q --ff-only session-c && git push -q; git checkout session-c -q
du -sh "$d"; df -h ~ | tail -n 1
