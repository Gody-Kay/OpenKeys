#!/usr/bin/env bash
# OpenKeys one-shot setup for Ubuntu/Debian/Kali. Run once from anywhere:
#   bash tools/setup.sh
# It installs build deps, fetches JUCE, builds everything, runs all tests.
# Skip steps with env vars: SKIP_APT=1  SKIP_JUCE=1
# Must be executed, never sourced (a failed step would close your terminal).
if [ -n "${ZSH_VERSION:-}" ] || [ "${BASH_SOURCE[0]}" != "$0" ]; then
  echo "Run it as:  bash tools/setup.sh   (do not 'source' it)"
  return 1 2>/dev/null || exit 1
fi

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build"
LOG="$ROOT/setup.log"
cd "$ROOT"
: > "$LOG"

step() { printf '\n== %s ==\n' "$*" | tee -a "$LOG"; }
run()  { "$@" >>"$LOG" 2>&1; }
die()  { echo "FAILED: $*  (see $LOG, last lines below)"; tail -n 20 "$LOG"; exit 1; }

SUDO=""
if [ "$(id -u)" -ne 0 ]; then SUDO="sudo"; fi

# 1. System packages
if [ "${SKIP_APT:-0}" != "1" ]; then
  step "Installing system packages (needs sudo)"
  command -v apt-get >/dev/null || die "apt-get not found: this script supports Debian/Ubuntu/Kali"
  run $SUDO apt-get update -o Acquire::IndexTargets::deb::Contents-deb::DefaultEnabled=false -o Acquire::Languages=none || die "apt-get update"
  PKGS="build-essential cmake ninja-build git python3 pkg-config curl
        libasound2-dev libjack-jackd2-dev
        libx11-dev libxext-dev libxinerama-dev libxrandr-dev libxcursor-dev libxcomposite-dev
        libfreetype-dev libfontconfig1-dev libgl1-mesa-dev libcurl4-openssl-dev"
  # shellcheck disable=SC2086
  run $SUDO env DEBIAN_FRONTEND=noninteractive apt-get install -y $PKGS || die "apt-get install"
  # sfizz is optional here: only used from Phase 1. Not fatal if the distro lacks it.
  if run $SUDO env DEBIAN_FRONTEND=noninteractive apt-get install -y libsfizz-dev; then
    echo "sfizz dev package installed" | tee -a "$LOG"
  else
    echo "NOTE: libsfizz-dev not in your distro; sfizz will be built from source in Phase 1." | tee -a "$LOG"
  fi
else
  step "Skipping system packages (SKIP_APT=1)"
fi

# 2. Configure (fetches JUCE from GitHub unless skipped)
JUCE_FLAG="-DOPENKEYS_WITH_JUCE=ON"
if [ "${SKIP_JUCE:-0}" = "1" ]; then JUCE_FLAG="-DOPENKEYS_WITH_JUCE=OFF"; fi
step "Configuring ($JUCE_FLAG)"
run cmake -S . -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release "$JUCE_FLAG" || die "cmake configure"

# 3. Build
step "Building"
run cmake --build "$BUILD" || die "build"

# 4. Tests (smoke, repo/schema, engine offline render)
step "Running tests"
run ctest --test-dir "$BUILD" --output-on-failure || die "tests"
ctest --test-dir "$BUILD" 2>&1 | tail -n 8

# 5. Sound tool check + engine benchmark on THIS machine
step "Sound tool"
python3 -I tools/fetch_sounds.py --list
step "Engine benchmark on this machine (copy the line into PROGRESS.md)"
"$BUILD/src/engine/engine_render_test" "$BUILD/render_test.wav" | tail -n 3
echo "CPU: $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2 | xargs)  RAM: $(free -m | awk '/Mem:/{print $2" MB"}')"

step "DONE. Test WAV: $BUILD/render_test.wav  (play it: aplay $BUILD/render_test.wav)"
