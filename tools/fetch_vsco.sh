#!/usr/bin/env bash
# Download VSCO 2 Community Edition (CC0, ~2.3 GB, SFZ branch) in small chunks that survive network drops.
# Needs ~6 GB free. Re-run any time: it resumes. Then run: bash tools/finish_vsco.sh
cd "$(dirname "$0")/.." || exit 1
d=content/sounds/vsco
mkdir -p content/sounds
[ -d "$d/.git" ] || [ -f "$d/.source-commit" ] || git clone -q --filter=blob:none --no-checkout --branch SFZ https://github.com/sgossner/VSCO-2-CE.git "$d"
[ -d "$d/.git" ] || { echo "already finished (no .git): $d"; exit 0; }
cd "$d" || exit 1
while true; do
  git rev-list --objects --missing=print HEAD | grep '^?' | cut -c2- > /tmp/vsco_missing.txt
  n=$(wc -l < /tmp/vsco_missing.txt); [ "$n" -eq 0 ] && break
  echo "files still missing: $n  ($(date +%H:%M))"
  head -n 150 /tmp/vsco_missing.txt | git -c fetch.negotiationAlgorithm=noop fetch origin --no-tags --no-write-fetch-head --recurse-submodules=no --filter=blob:none --stdin -q || { echo "chunk failed, retrying in 15s"; sleep 15; }
done
git checkout -f HEAD -- . && echo DOWNLOAD_COMPLETE
