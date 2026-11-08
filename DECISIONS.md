# DECISIONS (one line each; newest last; "assumed" = change it if you disagree)

- D001 Language/framework: C++17 + JUCE (real-time audio, one codebase for Linux/Windows/Mac). Python only for tools.
- D002 Licence: open source (assumed). JUCE is free for open-source use under AGPLv3, so OpenKeys will be AGPLv3/GPL. Anyone can use it free.
- D003 Sample engine: sfizz (SFZ). FluidSynth optional for SF2 soundfonts.
- D004 Platform order: Linux first (Kali/Ubuntu with PipeWire/JACK), then Windows (ASIO/WASAPI), then Mac (CoreAudio).
- D005 Working title: OpenKeys (assumed; rename later).
- D006 Own names in UI: Master Knob, Scenes, Setlist, Parts. No Yamaha/Roland names, artwork or copied layouts.
- D007 Sounds: free-to-redistribute licences only (CC0, CC-BY, public domain). Log each library in `content/LICENSES.md`. Never bundle Yamaha/Montage samples or recordings made from them.
- D008 Real instruments recorded by the church's engineer are 100% ours and the best content; target them for Rhodes-style EP, piano, organ, strings, choir.
- D009 Target hardware: 4 GB RAM, Core i5, no GPU. 64-voice default cap, Eco mode, shared reverbs, streamed samples.
- D010 V1 scope = Phases 0 to 5. Church extras, studio, arranger follow as updates.
- D011 Unsigned installers at first (no budget); users click through the "unknown developer" warning.
- D012 Mixer drum-processing is a SEPARATE project and repo. Do not mix code.
- D013 JUCE pinned to 8.0.4 via CMake FetchContent; fetched only once src/engine or src/app has a CMakeLists.txt (assumed).
- D014 CI: Linux must be green; Windows and Mac jobs may fail (continue-on-error) until tested by a friend (assumed).
- D015 Lane C tests are Python (jsonschema) registered in CTest: schema validation and licence check (assumed).
- 2026-11-06 (C, assumed): instrument.ref is relative to content/sounds/. First preset: content/presets/grand-piano.json.
- 2026-11-08 (C): removed continue-on-error from ci.yml; all 3 OS were green, so all 3 must stay green.
