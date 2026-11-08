// Phase 1 engine core: 16 Parts (zones/layers/splits), shared 64-voice pool,
// seamless patch switching, sustain + half-pedal, panic. JUCE-free.
#pragma once
#include <array>
#include <cstddef>
#include <atomic>
#include "EngineAPI.h"
#include "Chorus.h"
#include "Limiter.h"
#include "Reverb.h"
#include "StereoDelay.h"

namespace openkeys {

class PartEngine final : public IEngine {
public:
    PartEngine();

    void prepare(double sampleRate, int maxBlockSize) override;
    void process(float* left, float* right, int numSamples,
                 const MidiEvent* events, int numEvents) override;
    void panic() override { panicFlag_.store(true, std::memory_order_release); }
    void setMasterTuneHz(float hz) override { tuneHz_.store(hz, std::memory_order_relaxed); }
    void setMasterVolume(float v) override { masterVol_.store(v < 0.f ? 0.f : (v > 1.5f ? 1.5f : v), std::memory_order_relaxed); }
    void setEco(bool eco) override { eco_.store(eco, std::memory_order_relaxed); }
    void setPart(int index, const PartConfig& cfg) override;
    int activeVoices() const override { return active_.load(std::memory_order_relaxed); }
    int partVoices(int index) const override {
        return (index >= 0 && index < kMaxParts) ? partCount_[static_cast<size_t>(index)].load(std::memory_order_relaxed) : 0;
    }

private:
    enum class Stage : uint8_t { Off, Attack, Held, Release };

    // A voice copies everything it needs at note-on, so later setPart() calls
    // never change a note that is already sounding.
    struct Voice {
        Stage stage = Stage::Off;
        uint8_t part = 0, key = 0, channel = 0;
        bool sustained = false;     // key is up but the pedal holds it
        Waveform wf = Waveform::Sine;
        float gl = 0.f, gr = 0.f;   // velocity * volume * pan
        float sendRev = 0.f, sendDel = 0.f, sendCho = 0.f;  // velocity * volume * send amount
        float env = 0.f, attackStep = 0.f, relBase = 0.f, relStep = 0.f;
        double phase = 0.0, inc = 0.0;
        uint32_t age = 0;
    };

    struct Cmd { int index; PartConfig cfg; };
    static constexpr uint32_t kCmdCap = 64;  // power of two

    void applyCommands();
    void handle(const MidiEvent& e);
    void noteOn(uint8_t ch, uint8_t key, uint8_t vel);
    void startVoice(int part, uint8_t ch, uint8_t key, uint8_t vel);
    void noteOff(uint8_t ch, uint8_t key);
    void pedal(uint8_t value);
    void beginRelease(Voice& v, float scale);
    void killAll();

    std::array<PartConfig, kMaxParts> parts_{};
    std::array<Voice, kMaxVoices> voices_{};
    std::array<Cmd, kCmdCap> cmds_{};
    std::atomic<uint32_t> cmdHead_{0}, cmdTail_{0};  // SPSC: message thread -> audio thread

    double sampleRate_ = 48000.0;
    uint32_t counter_ = 0;
    float pedal_ = 0.f;  // 0..1, one pedal for all channels
    std::atomic<bool> panicFlag_{false};
    std::atomic<float> tuneHz_{440.f};
    std::atomic<float> masterVol_{1.f};
    std::atomic<bool> eco_{false};
    bool ecoApplied_ = false;
    dsp::Reverb reverb_;
    dsp::StereoDelay delay_;
    dsp::Chorus chorus_;
    dsp::Limiter limiter_;
    std::atomic<int> active_{0};
    std::array<std::atomic<int>, kMaxParts> partCount_{};
};

}  // namespace openkeys
