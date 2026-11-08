#!/usr/bin/env bash
# Download the Salamander Grand Piano V3 (CC-BY-3.0, ~1.4 GB, ~700 MB after .git is removed). Run: bash tools/fetch_salamander.sh
cd "$(dirname "$0")/.." || exit 1
d=content/sounds/salamander
[ -d "$d" ] && { echo "already present: $d"; exit 0; }
mkdir -p content/sounds
for i in 1 2 3 4 5; do
  git clone --depth 1 https://github.com/sfzinstruments/SalamanderGrandPiano.git "$d" && break
  echo "try $i failed, retrying in 10s"; rm -rf "$d"; sleep 10
done
[ -d "$d/.git" ] && { git -C "$d" rev-parse --short HEAD > "$d/.source-commit"; rm -rf "$d/.git"; }
echo "wav/flac files: $(find "$d" -type f \( -name '*.flac' -o -name '*.wav' \) | wc -l)"; du -sh "$d"
