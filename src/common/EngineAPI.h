// OpenKeys engine interface. DRAFT v1, owned by lane A. No JUCE types here.
// Other lanes talk to the engine only through this header.
// v1 (additive): PartConfig, setPart(), partVoices().
// v2 (additive): PartConfig sends, setMasterVolume(), setEco().
#pragma once
#include <cstdint>

namespace openkeys {

constexpr int kMaxVoices = 64;   // default voice cap (D009)
constexpr int kMaxParts = 16;

// Raw MIDI short message with its position inside the current audio block.
struct MidiEvent {
    uint32_t sampleOffset;  // 0 .. blockSize-1
    uint8_t status;         // e.g. 0x90 note on, 0x80 note off, 0xB0 control change
    uint8_t data1;
    uint8_t data2;
};

enum class Waveform : uint8_t { Sine, Triangle, Saw, Square };       // placeholder sources until sfizz/synths
enum class VelocityCurve : uint8_t { Soft, Normal, Hard };

// One Part = a zone (which notes it answers) + a sound + a mini channel strip.
struct PartConfig {
    bool enabled = true;
    uint8_t keyLow = 0, keyHigh = 127;     // split points
    uint8_t velLow = 1, velHigh = 127;     // velocity range (layer by touch)
    uint8_t channel = 0;                   // 0 = omni, 1..16
    int8_t transpose = 0;                  // semitones
    int8_t octave = 0;
    Waveform waveform = Waveform::Sine;
    VelocityCurve velocityCurve = VelocityCurve::Normal;
    float volume = 0.8f;                   // 0..1
    float pan = 0.f;                       // -1 left .. +1 right
    float attackSec = 0.005f;
    float releaseSec = 0.06f;
    float sendReverb = 0.f;                // 0..1, post-fader, shared reverb
    float sendDelay = 0.f;                 // 0..1, shared ping-pong delay
    float sendChorus = 0.f;                // 0..1, shared chorus
};

class IEngine {
public:
    virtual ~IEngine() = default;

    // Message thread. Allocates; never call from the audio thread.
    virtual void prepare(double sampleRate, int maxBlockSize) = 0;

    // Audio thread only: no allocation, no locks, no logging.
    // Events must be sorted by sampleOffset. Output is overwritten.
    virtual void process(float* left, float* right, int numSamples,
                         const MidiEvent* events, int numEvents) = 0;

    // Safe from any thread (applied at the start of the next block).
    virtual void panic() = 0;
    virtual void setMasterTuneHz(float hz) = 0;  // default 440
    virtual void setMasterVolume(float v) = 0;   // 0..1.5, default 1
    virtual void setEco(bool eco) = 0;           // cheaper reverb, chorus off (weak laptops)

    // Message thread (single producer). Applied at the start of the next block.
    // Notes already sounding keep their old sound (seamless switching).
    virtual void setPart(int index, const PartConfig& cfg) = 0;

    // Read-only meters, safe from any thread.
    virtual int activeVoices() const = 0;
    virtual int partVoices(int index) const = 0;
};

}  // namespace openkeys
