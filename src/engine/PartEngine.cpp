#include "PartEngine.h"
#include <algorithm>
#include <cmath>

namespace openkeys {

namespace {
constexpr double kTwoPi = 6.283185307179586476925286766559;
constexpr float kVoiceScale = 0.125f;  // headroom; tanh keeps the sum below 1 until the Phase 2 limiter

inline float wave(Waveform w, double p) {
    switch (w) {
        case Waveform::Sine:     return static_cast<float>(std::sin(p * kTwoPi));
        case Waveform::Triangle: return static_cast<float>(1.0 - 4.0 * std::fabs(p - 0.5));
        case Waveform::Saw:      return static_cast<float>(2.0 * p - 1.0) * 0.6f;
        case Waveform::Square:   return p < 0.5 ? 0.5f : -0.5f;
    }
    return 0.f;
}

inline float curve(VelocityCurve c, float v) {
    switch (c) {
        case VelocityCurve::Soft: return std::pow(v, 0.6f);
        case VelocityCurve::Hard: return std::pow(v, 1.6f);
        default:                  return v;
    }
}

// Half-pedal: the more the pedal is down (below the 50% hold point), the longer a released note rings.
inline float halfPedalScale(float p) { return p > 0.f ? 1.f / (1.f + 12.f * p) : 1.f; }
}  // namespace

PartEngine::PartEngine() {
    for (size_t i = 1; i < parts_.size(); ++i) parts_[i].enabled = false;  // Part 1 on, others off
    for (auto& c : partCount_) c.store(0);
}

void PartEngine::prepare(double sampleRate, int /*maxBlockSize*/) {
    sampleRate_ = sampleRate;
    reverb_.prepare(sampleRate);
    delay_.prepare(sampleRate);
    chorus_.prepare(sampleRate);
    limiter_.prepare(sampleRate);
    killAll();  // part settings are kept
}

void PartEngine::killAll() {
    for (auto& v : voices_) v = Voice{};
    pedal_ = 0.f;
    reverb_.reset();   // panic also silences the effect tails
    delay_.reset();
    chorus_.reset();
    limiter_.reset();
    active_.store(0, std::memory_order_relaxed);
    for (auto& c : partCount_) c.store(0, std::memory_order_relaxed);
}

void PartEngine::setPart(int index, const PartConfig& cfg) {
    if (index < 0 || index >= kMaxParts) return;
    const uint32_t head = cmdHead_.load(std::memory_order_relaxed);
    if (head - cmdTail_.load(std::memory_order_acquire) >= kCmdCap) return;  // queue full: drop
    cmds_[head % kCmdCap] = {index, cfg};
    cmdHead_.store(head + 1, std::memory_order_release);
}

void PartEngine::applyCommands() {
    uint32_t tail = cmdTail_.load(std::memory_order_relaxed);
    const uint32_t head = cmdHead_.load(std::memory_order_acquire);
    while (tail != head) {
        const Cmd& c = cmds_[tail % kCmdCap];
        parts_[static_cast<size_t>(c.index)] = c.cfg;
        ++tail;
    }
    cmdTail_.store(tail, std::memory_order_release);
}

void PartEngine::beginRelease(Voice& v, float scale) {
    v.stage = Stage::Release;
    v.sustained = false;
    v.relStep = v.relBase * scale;
}

void PartEngine::startVoice(int p, uint8_t ch, uint8_t key, uint8_t vel) {
    const PartConfig& c = parts_[static_cast<size_t>(p)];
    const int sounding = key + c.transpose + 12 * c.octave;
    if (sounding < 0 || sounding > 127) return;

    Voice* t = nullptr;
    for (auto& v : voices_)                                   // same key again: reuse, no click
        if (v.stage != Stage::Off && v.part == p && v.key == key && v.channel == ch) { t = &v; break; }
    if (!t)
        for (auto& v : voices_) if (v.stage == Stage::Off) { t = &v; break; }
    if (!t) {                                                 // steal: quietest releasing voice, else oldest
        Voice* rel = nullptr;
        for (auto& v : voices_)
            if (v.stage == Stage::Release && (!rel || v.env < rel->env)) rel = &v;
        if (rel) t = rel;
        else {
            t = &voices_[0];
            for (auto& v : voices_) if (v.age < t->age) t = &v;
        }
    }

    const double hz = static_cast<double>(tuneHz_.load(std::memory_order_relaxed)) *
                      std::pow(2.0, (sounding - 69) / 12.0);
    const float amp = curve(c.velocityCurve, static_cast<float>(vel) / 127.f) * c.volume;
    const float theta = (std::max(-1.f, std::min(1.f, c.pan)) + 1.f) * 0.7853981634f;  // 0..pi/2
    constexpr float kSqrt2 = 1.41421356f;  // centre pan = unity gain

    const bool fresh = t->stage == Stage::Off;
    t->stage = Stage::Attack;
    t->part = static_cast<uint8_t>(p);
    t->key = key;
    t->channel = ch;
    t->sustained = false;
    t->wf = c.waveform;
    t->gl = amp * std::cos(theta) * kSqrt2;
    t->gr = amp * std::sin(theta) * kSqrt2;
    t->sendRev = amp * std::max(0.f, std::min(1.f, c.sendReverb));
    t->sendDel = amp * std::max(0.f, std::min(1.f, c.sendDelay));
    t->sendCho = amp * std::max(0.f, std::min(1.f, c.sendChorus));
    t->attackStep = static_cast<float>(1.0 / (std::max(c.attackSec, 0.0005f) * sampleRate_));
    t->relBase = static_cast<float>(1.0 / (std::max(c.releaseSec, 0.002f) * sampleRate_));
    t->relStep = t->relBase;
    t->inc = hz / sampleRate_;
    t->age = ++counter_;
    if (fresh) { t->env = 0.f; t->phase = 0.0; }  // reuse/steal keep phase+env: no click
}

void PartEngine::noteOn(uint8_t ch, uint8_t key, uint8_t vel) {
    for (int p = 0; p < kMaxParts; ++p) {
        const PartConfig& c = parts_[static_cast<size_t>(p)];
        if (!c.enabled) continue;
        if (c.channel != 0 && c.channel != ch) continue;
        if (key < c.keyLow || key > c.keyHigh) continue;
        if (vel < c.velLow || vel > c.velHigh) continue;
        startVoice(p, ch, key, vel);
    }
}

void PartEngine::noteOff(uint8_t ch, uint8_t key) {
    for (auto& v : voices_) {
        if ((v.stage != Stage::Attack && v.stage != Stage::Held) || v.sustained) continue;
        if (v.key != key || v.channel != ch) continue;
        if (pedal_ >= 0.5f) v.sustained = true;                 // pedal holds the note
        else beginRelease(v, halfPedalScale(pedal_));            // light pedal = longer tail
    }
}

void PartEngine::pedal(uint8_t value) {
    const bool wasDown = pedal_ >= 0.5f;
    pedal_ = static_cast<float>(value) / 127.f;
    if (wasDown && pedal_ < 0.5f)
        for (auto& v : voices_)
            if (v.sustained) beginRelease(v, halfPedalScale(pedal_));
}

void PartEngine::handle(const MidiEvent& e) {
    const uint8_t type = e.status & 0xF0;
    const uint8_t ch = static_cast<uint8_t>((e.status & 0x0F) + 1);
    if (type == 0x90 && e.data2 > 0) noteOn(ch, e.data1, e.data2);
    else if (type == 0x80 || type == 0x90) noteOff(ch, e.data1);
    else if (type == 0xB0) {
        if (e.data1 == 64) pedal(e.data2);
        else if (e.data1 == 123)                                   // all notes off
            for (auto& v : voices_) if (v.stage != Stage::Off) beginRelease(v, 1.f);
    }
}

void PartEngine::process(float* left, float* right, int numSamples,
                         const MidiEvent* events, int numEvents) {
    if (panicFlag_.exchange(false, std::memory_order_acq_rel)) killAll();
    applyCommands();
    const bool eco = eco_.load(std::memory_order_relaxed);
    if (eco != ecoApplied_) { reverb_.setEco(eco); ecoApplied_ = eco; }
    const float master = masterVol_.load(std::memory_order_relaxed);

    int ev = 0;
    for (int i = 0; i < numSamples; ++i) {
        while (ev < numEvents && events[ev].sampleOffset <= static_cast<uint32_t>(i))
            handle(events[ev++]);

        float l = 0.f, r = 0.f, busRev = 0.f, busDel = 0.f, busCho = 0.f;
        for (auto& v : voices_) {
            if (v.stage == Stage::Off) continue;
            if (v.stage == Stage::Attack) {
                v.env += v.attackStep;
                if (v.env >= 1.f) { v.env = 1.f; v.stage = Stage::Held; }
            } else if (v.stage == Stage::Release) {
                v.env -= v.relStep;
                if (v.env <= 0.f) { v.env = 0.f; v.stage = Stage::Off; continue; }
            }
            const float s = wave(v.wf, v.phase) * v.env;
            l += s * v.gl;
            r += s * v.gr;
            busRev += s * v.sendRev;
            busDel += s * v.sendDel;
            busCho += s * v.sendCho;
            v.phase += v.inc;
            if (v.phase >= 1.0) v.phase -= 1.0;
        }
        // Shared send effects (one reverb/delay/chorus for all Parts: saves CPU).
        float rl = 0.f, rr = 0.f, dl = 0.f, dr = 0.f, cl = 0.f, cr = 0.f;
        reverb_.process(busRev * kVoiceScale, rl, rr);
        delay_.process(busDel * kVoiceScale, dl, dr);
        if (!eco) chorus_.process(busCho * kVoiceScale, cl, cr);
        float outL = (l * kVoiceScale + rl + dl + cl) * master;
        float outR = (r * kVoiceScale + rr + dr + cr) * master;
        limiter_.process(outL, outR);  // master chain: brickwall at 0.98
        left[i] = outL;
        right[i] = outR;
    }
    while (ev < numEvents) handle(events[ev++]);

    int n = 0;
    std::array<int, kMaxParts> per{};
    for (const auto& v : voices_)
        if (v.stage != Stage::Off) { ++n; ++per[v.part]; }
    active_.store(n, std::memory_order_relaxed);
    for (size_t p = 0; p < per.size(); ++p) partCount_[p].store(per[p], std::memory_order_relaxed);
}

}  // namespace openkeys
