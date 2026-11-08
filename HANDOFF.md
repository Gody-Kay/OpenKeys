# HANDOFF (read first, then SESSION_RULES.md, then the last 40 lines of PROGRESS.md)
- Project: OpenKeys, free open-source keyboard/studio instrument (C++17 + JUCE, AGPLv3). Lanes A Engine, B UI, C Content/Build.
- WORK ONLY IN ~/OpenKeys (the GitHub repo). ~/openkeys (lowercase) is an old backup of the engine, do not use it.
- User is a student almost out of tokens: replies max 5 lines, act first, no tutorials. Laptop: Kali Linux, zsh, i5-8365U, 7.7 GB. No MIDI keyboard (uses VMPK). Give ONE command block per step. Never tell them to source scripts.
- State: engine Phase 1 + 2 part 1 on main (16 Parts, splits/layers, pedals, reverb, delay, chorus, limiter, sends, Eco). CI green on Linux, Windows and macOS. ctest 6/6.
- Latency: pw-jack + JACK 48 kHz buffer 128 = about 2.7 ms. Shared ALSA is about 67 ms (tests only).
- Sounds: engine still plays sine/saw placeholders. Salamander Grand Piano V3 (CC-BY-3.0) is already downloaded to content/sounds/salamander (gitignored) and logged in content/LICENSES.md. Preset: content/presets/grand-piano.json.
## Next tasks (one at a time)
1. Integrate sfizz (BSD-2; add its licence line to content/LICENSES.md first) and play the Salamander SFZ. Add an offline render test in tests/engine. RAM under 1.5 GB.
2. Finish Phase 2: per-Part inserts (EQ, compressor, filter, drive, rotary), master EQ + compressor, level meters.
3. Performance JSON loader against docs/performance.schema.json + Scenes (lane A must sign off).
4. Expression pedal, velocity curves from JSON, voice limit + CPU meter.
5. Lane B UI later. Rule D020: whole Performance screen on ONE screen, no scrolling. Mockup (private to the user): https://claude.ai/artifact/NGhXUXVmJsz6q3KtD6d2dS
## Each session
Update PROGRESS.md, tick FEATURES.md only for tested items, log decisions in DECISIONS.md, commit small. Never claim done without build + test output. Say what was NOT verified (real sound, Win/Mac audio).
