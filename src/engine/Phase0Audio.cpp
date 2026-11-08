// Phase 0 console app: USB MIDI keyboard -> SineEngine -> sound card, prints latency.
// Lane A. JUCE is used only here (the engine itself stays JUCE-free).
//
//   openkeys_phase0 --list            list audio + MIDI devices and exit
//   openkeys_phase0 [--buffer 128] [--seconds N]
//
// MIDI hot-plug: devices are rescanned every second.
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_events/juce_events.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory>
#include <vector>

#include "PartEngine.h"

using namespace openkeys;

namespace {

std::atomic<bool> gStop{false};
void onSignal(int) { gStop.store(true); }

constexpr float kDefaultGain = 3.0f;  // test-app output gain (hard-clamped to +-1); --gain N overrides

// MIDI callbacks may come from several device threads, so writers share a spin lock
// (they only ever contend with each other). The audio thread is the single reader and
// never takes a lock.
class MidiQueue {
public:
    struct Item { MidiEvent ev; double tMs; };

    void push(const Item& it) {
        const juce::SpinLock::ScopedLockType lock(writeLock_);
        int s1, n1, s2, n2;
        fifo_.prepareToWrite(1, s1, n1, s2, n2);
        if (n1 + n2 < 1) return;  // full: drop
        buf_[static_cast<size_t>(n1 > 0 ? s1 : s2)] = it;
        fifo_.finishedWrite(1);
    }

    bool pop(Item& it) {  // audio thread only
        int s1, n1, s2, n2;
        fifo_.prepareToRead(1, s1, n1, s2, n2);
        if (n1 + n2 < 1) return false;
        it = buf_[static_cast<size_t>(n1 > 0 ? s1 : s2)];
        fifo_.finishedRead(1);
        return true;
    }

private:
    static constexpr int kCap = 1024;
    juce::AbstractFifo fifo_{kCap};
    std::array<Item, kCap> buf_{};
    juce::SpinLock writeLock_;
};

class App final : public juce::AudioIODeviceCallback, public juce::MidiInputCallback {
public:
    PartEngine engine;
    float outGain = kDefaultGain;  // set before the audio callback is added
    std::atomic<int> notesReceived{0};
    std::atomic<float> latAvgMs{0.f}, latMaxMs{0.f};

    // ---- audio thread: no allocation, no locks, no logging ----
    void audioDeviceIOCallbackWithContext(const float* const*, int, float* const* out, int numOut,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext&) override {
        constexpr int kMaxEv = 256;
        MidiEvent evs[kMaxEv];
        int ne = 0;
        MidiQueue::Item it;
        const double now = juce::Time::getMillisecondCounterHiRes();
        while (ne < kMaxEv && queue_.pop(it)) {
            evs[ne++] = it.ev;
            const float d = static_cast<float>(now - it.tMs);
            latAvgMs.store(latAvgMs.load(std::memory_order_relaxed) * 0.9f + d * 0.1f,
                           std::memory_order_relaxed);
            if (d > latMaxMs.load(std::memory_order_relaxed))
                latMaxMs.store(d, std::memory_order_relaxed);
        }

        const int cap = static_cast<int>(scratchL_.size());
        int done = 0;
        while (done < numSamples) {
            const int c = std::min(numSamples - done, cap);
            engine.process(scratchL_.data(), scratchR_.data(), c, done == 0 ? evs : nullptr,
                           done == 0 ? ne : 0);
            for (int ch = 0; ch < numOut; ++ch) {
                if (!out[ch]) continue;
                const float* src = ch == 0 ? scratchL_.data() : ch == 1 ? scratchR_.data() : nullptr;
                float* dst = out[ch] + done;
                if (src) for (int i = 0; i < c; ++i) dst[i] = std::max(-1.f, std::min(1.f, src[i] * outGain));
                else std::memset(dst, 0, sizeof(float) * static_cast<size_t>(c));
            }
            done += c;
        }
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* dev) override {
        const int block = dev->getCurrentBufferSizeSamples();
        scratchL_.assign(static_cast<size_t>(std::max(block, 4096)), 0.f);
        scratchR_.assign(scratchL_.size(), 0.f);
        engine.prepare(dev->getCurrentSampleRate(), block);
    }
    void audioDeviceStopped() override {}

    // ---- MIDI device threads ----
    void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage& m) override {
        const int size = m.getRawDataSize();
        const juce::uint8* d = m.getRawData();
        if (size < 1 || d[0] < 0x80 || d[0] >= 0xF0) return;  // channel messages only
        MidiQueue::Item it;
        it.ev = {0, d[0], static_cast<uint8_t>(size > 1 ? d[1] : 0),
                 static_cast<uint8_t>(size > 2 ? d[2] : 0)};
        it.tMs = juce::Time::getMillisecondCounterHiRes();
        if ((d[0] & 0xF0) == 0x90 && size > 2 && d[2] > 0) notesReceived.fetch_add(1);
        queue_.push(it);
    }

private:
    MidiQueue queue_;
    std::vector<float> scratchL_, scratchR_;
};

// Try-out setups for the on-screen piano: sine (default), split, layer, pad.
bool applyPreset(PartEngine& e, const juce::String& name) {
    PartConfig p0, p1;
    if (name == "sine") { p0.sendReverb = 0.2f; e.setPart(0, p0); return true; }
    if (name == "split") {          // left hand saw bass, right hand sine
        p0.keyHigh = 59; p0.waveform = Waveform::Saw; p0.octave = -1; p0.volume = 0.5f; p0.pan = -0.3f; p0.sendReverb = 0.1f;
        p1.keyLow = 60; p1.waveform = Waveform::Sine; p1.volume = 0.8f; p1.pan = 0.3f; p1.sendReverb = 0.3f; p1.sendDelay = 0.2f;
        e.setPart(0, p0); e.setPart(1, p1);
        return true;
    }
    if (name == "layer") {          // sine + soft triangle pad an octave up
        p0.sendReverb = 0.25f;
        p1.waveform = Waveform::Triangle; p1.octave = 1; p1.volume = 0.35f;
        p1.attackSec = 0.15f; p1.releaseSec = 0.8f; p1.sendReverb = 0.4f; p1.sendChorus = 0.6f;
        e.setPart(0, p0); e.setPart(1, p1);
        return true;
    }
    if (name == "pad") {
        p0.waveform = Waveform::Triangle; p0.attackSec = 0.3f; p0.releaseSec = 1.2f;
        p0.sendReverb = 0.5f; p0.sendChorus = 0.5f;
        e.setPart(0, p0);
        return true;
    }
    return false;
}

void listDevices() {
    juce::AudioDeviceManager adm;
    std::printf("Audio backends:\n");
    for (auto* type : adm.getAvailableDeviceTypes()) {
        type->scanForDevices();
        std::printf("  [%s]\n", type->getTypeName().toRawUTF8());
        for (const auto& n : type->getDeviceNames(false))
            std::printf("    out: %s\n", n.toRawUTF8());
    }
    std::printf("MIDI inputs:\n");
    for (const auto& d : juce::MidiInput::getAvailableDevices())
        std::printf("  %s\n", d.name.toRawUTF8());
}

}  // namespace

int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI juceInit;  // gives JUCE a message thread

    int bufferSize = 128;
    int seconds = 0;  // 0 = until Ctrl-C
    juce::String deviceFilter;  // --device TEXT: only try devices whose name contains TEXT
    float gain = kDefaultGain;
    juce::String typeFilter, preset = "sine";  // --type ALSA|JACK, --preset sine|split|layer|pad
    double rate = 0;                           // --rate 48000 (0 = device default)
    bool eco = false;                          // --eco: cheaper reverb, chorus off
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--eco")) eco = true;
        if (!std::strcmp(argv[i], "--type") && i + 1 < argc) typeFilter = argv[++i];
        if (!std::strcmp(argv[i], "--preset") && i + 1 < argc) preset = argv[++i];
        if (!std::strcmp(argv[i], "--rate") && i + 1 < argc) rate = std::atof(argv[++i]);
        if (!std::strcmp(argv[i], "--gain") && i + 1 < argc) gain = static_cast<float>(std::atof(argv[++i]));
        if (!std::strcmp(argv[i], "--device") && i + 1 < argc) deviceFilter = argv[++i];
        if (!std::strcmp(argv[i], "--list")) { listDevices(); return 0; }
        if (!std::strcmp(argv[i], "--buffer") && i + 1 < argc) bufferSize = std::atoi(argv[++i]);
        if (!std::strcmp(argv[i], "--seconds") && i + 1 < argc) seconds = std::atoi(argv[++i]);
    }
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    App app;
    app.outGain = gain;
    if (!applyPreset(app.engine, preset)) {
        std::printf("Unknown preset '%s' (use sine, split, layer or pad)\n", preset.toRawUTF8());
        return 1;
    }
    app.engine.setEco(eco);
    std::printf("Preset: %s%s\n", preset.toRawUTF8(), eco ? " (Eco)" : "");
    juce::AudioDeviceManager adm;
    // Try every output device (JACK last) until one opens with 2 channels; say why others fail.
    juce::AudioIODevice* dev = nullptr;
    {
        std::vector<std::pair<juce::String, juce::String>> candidates;  // type, device
        for (int pass = 0; pass < 2; ++pass)
            for (auto* type : adm.getAvailableDeviceTypes()) {
                if ((type->getTypeName() == "JACK") != (pass == 1)) continue;
                if (typeFilter.isNotEmpty() && type->getTypeName() != typeFilter) continue;
                type->scanForDevices();
                for (const auto& n : type->getDeviceNames(false))
                    if (deviceFilter.isEmpty() || n.containsIgnoreCase(deviceFilter))
                        candidates.emplace_back(type->getTypeName(), n);
            }
        for (const auto& c : candidates) {
            adm.setCurrentAudioDeviceType(c.first, true);
            juce::AudioDeviceManager::AudioDeviceSetup s;
            s.outputDeviceName = c.second;
            s.sampleRate = rate;
            s.bufferSize = bufferSize;
            s.useDefaultInputChannels = false;
            s.useDefaultOutputChannels = true;
            const juce::String e = adm.setAudioDeviceSetup(s, true);
            auto* d = adm.getCurrentAudioDevice();
            if (e.isEmpty() && d != nullptr && d->getActiveOutputChannels().countNumberOfSetBits() > 0) {
                dev = d;
                break;
            }
            std::printf("  skip [%s] %s: %s\n", c.first.toRawUTF8(), c.second.toRawUTF8(),
                        e.isEmpty() ? "no output channels" : e.toRawUTF8());
        }
    }
    if (!dev) {
        std::printf("Audio init failed: no usable output device.\nTry:  openkeys_phase0 --list\n");
        return 2;
    }
    adm.addAudioCallback(&app);

    const double sr = dev->getCurrentSampleRate();
    const int buf = dev->getCurrentBufferSizeSamples();
    const int outLat = dev->getOutputLatencyInSamples();
    std::printf("Audio: %s / %s, %.0f Hz\n", dev->getTypeName().toRawUTF8(),
                dev->getName().toRawUTF8(), sr);
    std::printf("Buffer %d samples = %.1f ms | driver-reported output latency %d samples = %.1f ms\n",
                buf, 1000.0 * buf / sr, outLat, 1000.0 * outLat / sr);
    std::printf("Estimated key-to-sound latency ~ %.1f ms (buffer + output; USB MIDI adds ~1 ms)\n",
                1000.0 * (buf + outLat) / sr);
    std::printf("Play your keyboard. Ctrl-C to quit.\n");

    std::map<juce::String, std::unique_ptr<juce::MidiInput>> inputs;
    auto rescan = [&] {
        const auto avail = juce::MidiInput::getAvailableDevices();
        for (auto it = inputs.begin(); it != inputs.end();) {
            const bool still = std::any_of(avail.begin(), avail.end(),
                [&](const juce::MidiDeviceInfo& d) { return d.identifier == it->first; });
            if (!still) { std::printf("MIDI disconnected\n"); it = inputs.erase(it); }
            else ++it;
        }
        for (const auto& d : avail) {
            if (inputs.count(d.identifier)) continue;
            if (auto in = juce::MidiInput::openDevice(d.identifier, &app)) {
                in->start();
                std::printf("MIDI connected: %s\n", d.name.toRawUTF8());
                inputs[d.identifier] = std::move(in);
            }
        }
    };

    int tick = 0;
    while (!gStop.load()) {
        if (tick % 10 == 0) rescan();  // every 1 s
        juce::Thread::sleep(100);
        ++tick;
        if (tick % 20 == 0)
            std::printf("voices %2d | notes %d | MIDI->audio thread avg %.2f ms, max %.2f ms\n",
                        app.engine.activeVoices(), app.notesReceived.load(),
                        static_cast<double>(app.latAvgMs.load()),
                        static_cast<double>(app.latMaxMs.exchange(0.f)));
        if (seconds > 0 && tick >= seconds * 10) break;
    }

    inputs.clear();
    adm.removeAudioCallback(&app);
    adm.closeAudioDevice();
    std::printf("Bye.\n");
    return 0;
}
