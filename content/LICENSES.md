# LICENSES (every dependency and sound library MUST have a line here before use)

Allowed for sounds: CC0, CC-BY, public domain, permissive. Never bundle Yamaha/Roland or other copyrighted samples.

## Code dependencies
| Name | Used for | Licence | Source | Notes |
|---|---|---|---|---|
| JUCE | UI, audio, MIDI, plugin formats | AGPLv3 (open-source use) | https://github.com/juce-framework/JUCE | Pinned 8.0.4 in CMakeLists.txt. Makes OpenKeys AGPLv3/GPL (D002). |
| sfizz | SFZ sample playback | BSD-2-Clause | https://github.com/sfztools/sfizz | Added in Phase 1. |
| FluidSynth | Optional SF2 playback | LGPL-2.1 | https://github.com/FluidSynth/fluidsynth | Optional. Link dynamically. |

## Sound libraries
| Name | Licence | Source URL | Attribution needed | Notes |
|---|---|---|---|---|
| Salamander Grand Piano V3 | CC-BY-3.0 | https://github.com/sfzinstruments/SalamanderGrandPiano | Yes: Alexander Holm, CC-BY 3.0 | Yamaha C5, 48kHz/24bit, 16 layers. Cloned commit 3382bf9. Uses SFZ v2 + ARIA extensions: test in sfizz in Phase 1. |
| VSCO 2 Community Edition (SFZ branch) | CC0-1.0 | https://github.com/sgossner/VSCO-2-CE | Credit requested, not required: Versilian Studios LLC / Sam Gossner | Commit 6dd651d. Strings, brass, woodwinds, percussion, keys. Readme asks not to resell as-is; the CC0 LICENSE governs. |

## Our own recordings
Recordings made by the church's engineer are owned by the project (D008). Log each set here with date and who recorded it.
