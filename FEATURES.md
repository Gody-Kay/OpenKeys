# FEATURES (tick when done and tested)

V1 = ships after Phase 4 + 5. Later = Phases 6 to 8.

## V1: Core
- [ ] Any USB MIDI keyboard works with no setup (auto-detect, hot-plug)
- [ ] First-run audio wizard with latency test
- [ ] 16 Parts per Performance
- [ ] Layers (up to 16) and splits with set split points; key and velocity ranges per Part
- [ ] Seamless switching: held notes keep ringing when the sound changes
- [ ] Sustain with half-pedal, expression pedal, footswitch for next/previous patch or scene
- [ ] Master transpose, octave shift, master tune; one-tap Reset (transpose 0, 440 Hz)
- [ ] Velocity curves per Part (soft, normal, hard)
- [ ] Panic button; audio engine auto-restart
- [ ] Mono output mode; stereo main out
- [ ] Voice limit and Eco mode (lowers quality automatically on weak laptops); CPU meter
- [ ] Auto-save, crash recovery, one-click backup/restore to USB

## V1: Performance control
- [ ] Performances (saved setups) with Scenes (8 per Performance)
- [ ] Scenes store Part on/off, volume, pan, sends, effects, arpeggio settings
- [ ] Animated faders and knobs glide to saved positions on recall (can be turned off)
- [ ] Glow effects on lit Part and Scene buttons (colour per Part)
- [ ] Master Knob: one control, many parameters, custom curves; assignable to pedal
- [ ] 4 assignable knobs + 4 sliders on screen; MIDI learn for hardware knobs
- [ ] Knob matching (hardware knob does not make the sound jump)
- [ ] Setlist: ordered list of Performances, no copying
- [ ] Patch browser: categories, search, favourites, recently used
- [ ] Registrations: 8 buttons per bank
- [ ] Touch lock; big-text mode; dark mode; multi-language interface

## V1: Sound and effects
- [ ] Sampled piano, strings, brass, orchestra, choir via sfizz
- [ ] Synth engines: FM electric piano (Suitcase-style), organ (drawbars + rotary), pads, subtractive leads/bass
- [ ] Piano extras: string resonance, pedal noise, key-release, round-robin
- [ ] Filters: low-pass, high-pass, band-pass, formant; resonance, envelope, LFO per Part
- [ ] Per-sound knobs: attack, release, brightness, cutoff; unison, detune, portamento
- [ ] Insert effects per Part (EQ, compressor, gate, drive, tremolo, chorus, phaser, rotary)
- [ ] Send effects: reverbs, delays, chorus; shared across Parts to save CPU
- [ ] Master chain: EQ, compressor, limiter, meters
- [ ] 150 tuned patches at launch, then growing to 700

## V1: Content and tools
- [ ] Sound download script (free licences only)
- [ ] `content/LICENSES.md` complete for every library
- [ ] Sound packs others can install (JSON manifest)
- [ ] Installers: Linux, Windows, Mac from CI

## Later: Church extras (Phase 6)
- [ ] Worship pads in every key with crossfade; pad continues if next song is same key
- [ ] Click track and guide cues on separate outputs (for in-ears)
- [ ] Lyrics, text and chord charts on a second screen / projector
- [ ] PDF chart display
- [ ] Song library: key, tempo, time signature, sounds, lyrics per song
- [ ] Backing track player with markers; keeps playing when patch changes
- [ ] Practice mode: metronome, on-screen keyboard, follow-the-lights lesson modes
- [ ] Chord helper and easy mode for beginners
- [ ] Remote control from phone/tablet on church Wi-Fi
- [ ] Auto-record every service (audio + MIDI)

## Later: Studio (Phase 7)
- [ ] Multitrack MIDI + audio recorder; export WAV and MIDI
- [ ] Mixer view: faders, mute/solo, sends, groups
- [ ] VST3 and CLAP plugin versions
- [ ] Multi-output routing, USB audio device selection
- [ ] Audio input as a Part (mics, other instruments) with effects

## Later: Arranger and extras (Phase 8)
- [ ] Arpeggiator (multi-Part) and motion sequencer
- [ ] Accompaniment styles: intro, variations, fills, break, ending
- [ ] Fingering modes (single finger, fingered, full keyboard)
- [ ] One Touch Settings (4 per style)
- [ ] Chord Looper (record and play chord sequences)
- [ ] Style Creator; convert a MIDI song into a style
- [ ] Multi pads (audio and MIDI)
- [ ] Registration sequence and freeze
- [ ] Vocal harmony, vocoder, mic effects (gate, compressor, EQ)
- [ ] Audio player with time-stretch, pitch-shift, vocal cancel
- [ ] Keyboard sampler and pad sampler; loop recorder; stem export from our own recordings
- [ ] Style/arpeggio import (open formats) and MIDI song playback with lyrics
