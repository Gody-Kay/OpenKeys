# PROGRESS (append at the end; newest last; sessions read the last 40 lines)

## Status
Phase 0 not started. No code yet.

## Benchmarks (record here)
- Laptop: (fill in) | Round-trip latency: (not measured) | 64 voices CPU: (not measured) | RAM: (not measured)

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
