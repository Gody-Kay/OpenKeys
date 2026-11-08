# PROGRESS (append at the end; newest last; sessions read the last 40 lines)

## Status
Phase 0 not started. No code yet.

## Benchmarks (record here)
- Laptop: Intel(R) Core(TM) i5-8365U CPU @ 1.60GHz, 8 threads, 7.6 GB RAM, Linux 6.19.14+kali-amd64 | Round-trip latency: (not measured) | 64 voices CPU: (not measured) | RAM: (not measured)

## Log
- [Planning] Plan, features, rules and decisions written. Next: start Phase 0.
- [C] Phase 0 files written: CMakeLists, CI workflow, performance.schema.json + example, LICENSES.md, check_licenses.py, fetch_sounds.py skeleton. Local ctest: 2/2 pass. CI on GitHub not yet run.

## Next up
- A: JUCE audio+MIDI skeleton, MIDI note -> sine, measure latency, draft `src/common/EngineAPI.h`
- B: `src/app` window with fake Performance screen (Part buttons, Scene buttons, 4 faders) using dummy data
- C: repo layout, CMakeLists, CI for Linux/Windows/Mac, `docs/performance.schema.json`, `content/LICENSES.md`, `tools/fetch_sounds.py` skeleton

## Blocked
(none)

## REQUESTS (to another lane; format: from -> to: what, why)
(none)

## Known issues
(none)
- [C] CI green on GitHub (Linux/Windows/Mac) after first push. Phase 0 lane C complete.
- [C] 2026-11-06 16:14 HANDOFF: lane C Phase 0 done, CI green, all branches pushed. Next session to start: A (Engine). Then B (UI). C idle until requests arrive.
- [C] 2026-11-06 16:18 fetch_sounds.py hardened: sha256 required, zip path check, skip if already downloaded, --hash helper. sources.json still empty.
- [C] 2026-11-06 17:12 Salamander Grand Piano V3 (CC-BY-3.0) downloaded to content/sounds/salamander (gitignored), logged in LICENSES.md.
- [C] 2026-11-06 17:14 tools/benchmark.py added; laptop specs recorded. Latency/64-voice/RAM still to measure once engine has audio.
- [C] 2026-11-06 17:18 content/presets/grand-piano.json added (Salamander). Ref paths are relative to content/sounds/ (see DECISIONS.md). A: please confirm this resolves in the engine.
- [C] 2026-11-06 17:20 tools/check_presets.py + ctest presets_valid added. CI still needs pip install jsonschema (pending).
- [C] 2026-11-06 17:21 CI: ci.yml already mentions jsonschema, left unchanged. Verify presets_valid runs (not skipped) on all 3 OS in the Actions log.
- [C] 2026-11-08 17:45 Engine Phase 1+2 imported from ~/openkeys into main (branch engine-import). ctest 6/6 pass locally. Engine's own PROGRESS/DECISIONS/FEATURES/setup.sh NOT imported yet. CI on 3 OS not yet checked.
- [C] 2026-11-08 21:04 CI GREEN on ubuntu-latest (first real run; earlier run was blocked by the repo's pin-actions-to-SHA setting, now off). Windows/macOS status not checked.
- [C] 2026-11-08 21:09 CI GREEN on Linux, Windows and macOS. continue-on-error removed. Real audio output untested on Win/Mac.
- [C] 2026-11-08 21:14 Imported HANDOFF.md (rewritten for ~/OpenKeys), tools/setup.sh, merged DECISIONS.md (my lines are now DC1-DC3).
- [C] 2026-11-08 21:17 ci.yml: checkout@v5, setup-python@v6 (Node 20 deprecation). Verify 3 OS still green.
- [C] 2026-11-08 22:13 Added tools/fetch_salamander.sh, tools/fetch_vsco.sh, docs/SOUNDS.md so sounds can be re-downloaded by anyone. Syntax-checked only; fetch scripts not re-run.
- [C->B] 2026-11-08 22:23 UI draft on branch ui-draft: src/app/{CMakeLists.txt,Main.cpp,PerformanceScreen.h}. DUMMY data, NOT compiled, NOT run. Needs: build with OPENKEYS_WITH_JUCE=ON, fix errors, then merge. Not connected to engine yet.
- [C->B] 2026-11-08 23:48 UI draft v2 on ui-draft: 3D hardware look, LCD, LAYERS button opening 16 layer faders, sound name replaces Layer N. Dummy data, not connected to engine. Compile/run NOT yet verified.
