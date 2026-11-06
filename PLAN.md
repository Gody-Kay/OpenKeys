# OpenKeys: Build Plan

**Goal:** a fast, light, easy-to-learn keyboard and studio instrument that takes any USB MIDI keyboard with no setup. Target: 9 to 9.5 out of 10 against Yamaha's Montage/MODX software for features and ease of use. Sound quality target: 8 to 8.5 on day one, rising as libraries and our own recordings improve.

**Runs on:** Linux first, then Windows and Mac, from one codebase. Free and open source.
**Benchmark machine:** 4 GB RAM, Core i5, no GPU. Everything must pass on it.

## Stack (see DECISIONS.md)
- C++17 + JUCE (UI, audio, MIDI, plugin formats: standalone first, VST3 and CLAP later)
- Sample playback: sfizz (SFZ format). Optional: FluidSynth (SF2).
- Our own code: synth engines (FM, subtractive, organ, pads), effects, mixer, arpeggiator, arranger.
- Data: JSON (Performances, Setlists, settings, sound-pack manifests).
- Tools and scripts: Python (sound download, license tracker, packaging helpers).
- Build/CI: CMake + GitHub Actions (Linux, Windows, Mac builds from one code).

## Architecture
```
MIDI in -> Router (zones: key range, velocity range, transpose, channel)
        -> Part 1..16 [Instrument: SFZ sampler | synth engine]
                      -> Insert FX chain (filter, EQ, comp, drive, rotary...)
                      -> Part mixer (volume, pan, sends)
        -> Send buses (reverb, delay, chorus)
        -> Master chain (EQ, compressor, limiter)
        -> Audio out (main L/R + extra outs for click/in-ears)
```
- **Performance** = one saved setup: up to 16 Parts, 8 Scenes, Master Knob map, effects, arpeggio settings. Stored as JSON.
- **Scene** = a snapshot of Part on/off, volumes, pans, sends, effect levels. Recalling one glides the faders and knobs (animation) while the sound morphs.
- **Master Knob** = one control mapped to many parameters, each with its own range and curve.
- Control thread (UI) talks to the audio thread through a lock-free queue. Every parameter has an ID, range, and smoothing.

## Lanes
- **A Engine:** audio I/O, MIDI router, Parts, sfizz integration, synth engines, DSP effects, voice limits, Eco mode.
- **B UI:** touch-first screens, faders/knobs with glide animation, glow effects on lit buttons, patch browser, setlist, on-screen keyboard, settings.
- **C Content & Build:** repo, CMake, CI for 3 OSes, sound download script, license tracker, preset JSON authoring, benchmarks, installers, docs.

**Interface contract:** `src/common/EngineAPI.h` (owned by A) and `docs/performance.schema.json` (owned by C, agreed with A). B only talks to the engine through these.

## Phases
| # | Phase | Lane lead | Exit test |
|---|---|---|---|
| 0 | Skeleton: repo, build on 3 OS, MIDI note -> sound, latency measured | A + C | Press a key, hear a tone; CI green on Linux |
| 1 | Engine core: Parts, layers, splits, sfizz piano, seamless patch switching, sustain/half-pedal, panic | A | 16 parts layered, no cut-off when switching |
| 2 | Effects: channel strip, reverb, delay, chorus, rotary, master chain, send buses, Eco mode | A | Test render has no NaN/clipping; CPU target met |
| 3 | Synth engines: FM electric piano (Suitcase-style), organ, pads, subtractive leads/bass, filters | A | 40 playable patches |
| 4 | UI v1: Performance screen, Part and Scene buttons, animated faders, Master Knob, browser, setlist, MIDI learn | B | Full Sunday flow works by touch only |
| 5 | Sound library: download script, license tracker, tuned patches (150 first, then 700) | C | Every patch has a licence line |
| 6 | Church extras: pads in every key, click + separate outs, lyrics on 2nd screen, song library | A + B + C | Song loads sounds, lyrics, click, pad together |
| 7 | Studio: recorder, multitrack, VST3/CLAP, mixer view, multi-out | A + B | Loads as plugin in a DAW |
| 8 | Arranger: styles, Chord Looper, One Touch Settings, multi pads, vocal harmony, samplers, loops | A + B + C | Play chords, accompaniment follows |
| 9 | Polish: benchmarks, installers, first-run wizard, docs, testers on 3 OSes | C | Installs and runs on a clean machine |

**Ship version 1 after Phase 4 + 5.** Phases 6 to 8 follow as updates.

## First tasks (Phase 0)
- **A:** JUCE audio+MIDI skeleton in `src/engine`; MIDI note -> sine; print measured round-trip latency; `EngineAPI.h` draft.
- **B:** `src/app` window with a fake Performance screen (Part buttons, Scene buttons, 4 faders) using dummy data from the schema.
- **C:** repo layout, CMakeLists, GitHub Actions for Linux/Windows/Mac, `docs/performance.schema.json`, `content/LICENSES.md`, `tools/fetch_sounds.py` skeleton.

## Risks
- Low latency on Windows needs ASIO or WASAPI; test early.
- Heavy reverb and big sample libraries can break the 4 GB target: stream samples, share reverbs, Eco mode.
- Sound licences: log before use. Never bundle Yamaha or other copyrighted samples.
- Mac and Windows can't be tested from Linux: ask a friend to test CI builds each phase.
