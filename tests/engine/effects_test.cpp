// Phase 2 tests: reverb / delay / chorus / limiter on their own, then Part sends, Eco, master volume,
// panic-clears-tails, and a CPU benchmark with every effect running. Offline, no audio device.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>
#include "Chorus.h"
#include "Limiter.h"
#include "PartEngine.h"
#include "Reverb.h"
#include "StereoDelay.h"

using namespace openkeys;

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { std::printf("FAIL: %s\n", msg); ++fails; } } while (0)

static constexpr double kSr = 48000.0;
static constexpr double kTwoPi = 6.283185307179586;

static double energy(const std::vector<float>& v, size_t a, size_t b) {
    double e = 0.0;
    for (size_t i = a; i < b && i < v.size(); ++i) e += static_cast<double>(v[i]) * v[i];
    return e;
}
static bool finite(const std::vector<float>& v) {
    for (float x : v) if (!std::isfinite(x)) return false;
    return true;
}

static MidiEvent on(uint8_t key, uint8_t vel) { return {0, 0x90, key, vel}; }
static MidiEvent off(uint8_t key) { return {0, 0x80, key, 0}; }

struct Rig {
    static constexpr int kBlock = 128;
    PartEngine e;
    std::vector<float> L = std::vector<float>(kBlock), R = std::vector<float>(kBlock);
    bool nan = false;
    float peak = 0.f;       // over everything rendered
    float lastPeak = 0.f;   // of the most recent block
    float lastDiff = 0.f;   // max |L-R| of the most recent block

    Rig() { e.prepare(kSr, kBlock); }
    void run(int blocks, std::vector<MidiEvent> first = {}) {
        for (int b = 0; b < blocks; ++b) {
            e.process(L.data(), R.data(), kBlock, b == 0 ? first.data() : nullptr,
                      b == 0 ? static_cast<int>(first.size()) : 0);
            lastPeak = 0.f; lastDiff = 0.f;
            for (int i = 0; i < kBlock; ++i) {
                if (!std::isfinite(L[i]) || !std::isfinite(R[i])) nan = true;
                lastPeak = std::fmax(lastPeak, std::fmax(std::fabs(L[i]), std::fabs(R[i])));
                lastDiff = std::fmax(lastDiff, std::fabs(L[i] - R[i]));
            }
            peak = std::fmax(peak, lastPeak);
        }
    }
    // Peak over a window of blocks.
    float peakOver(int blocks) {
        float p = 0.f;
        for (int b = 0; b < blocks; ++b) { run(1); p = std::fmax(p, lastPeak); }
        return p;
    }
    void untilVoicesEnd(int maxBlocks = 400) { for (int b = 0; b < maxBlocks && e.activeVoices() > 0; ++b) run(1); }
};

static PartConfig sends(float rev, float del, float cho) {
    PartConfig c; c.sendReverb = rev; c.sendDelay = del; c.sendChorus = cho; return c;
}

int main() {
    // 1. Reverb: tail exists, decays, dies out, stays finite and bounded on noise.
    {
        dsp::Reverb rv; rv.prepare(kSr);
        std::vector<float> L, R;
        for (int i = 0; i < static_cast<int>(5 * kSr); ++i) {
            float l, r; rv.process(i == 0 ? 1.f : 0.f, l, r); L.push_back(l); R.push_back(r);
        }
        const size_t s = static_cast<size_t>(kSr);
        const double e0 = energy(L, s / 20, s / 2), e1 = energy(L, s, s * 3 / 2), e2 = energy(L, 4 * s, 5 * s);
        std::printf("reverb: energy 0.05-0.5s %.3e | 1-1.5s %.3e | 4-5s %.3e\n", e0, e1, e2);
        CHECK(e0 > 1e-8, "reverb: impulse makes a tail");
        CHECK(e1 > 0.0 && e1 < e0 * 0.2, "reverb: tail decays");
        CHECK(e2 < 1e-9, "reverb: tail dies out");
        CHECK(finite(L) && finite(R), "reverb: finite");
        double d = 0; for (size_t i = 0; i < L.size(); ++i) d += std::fabs(L[i] - R[i]);
        CHECK(d > 1e-6, "reverb: stereo (L differs from R)");

        dsp::Reverb rn; rn.prepare(kSr);
        float pk = 0.f; unsigned seed = 12345; bool ok = true;
        for (int i = 0; i < static_cast<int>(10 * kSr); ++i) {
            seed = seed * 1664525u + 1013904223u;
            const float x = (static_cast<float>(seed >> 8) / 8388608.f) - 1.f;  // -1..1
            float l, r; rn.process(x, l, r);
            if (!std::isfinite(l) || !std::isfinite(r)) ok = false;
            pk = std::fmax(pk, std::fmax(std::fabs(l), std::fabs(r)));
        }
        CHECK(ok && pk < 10.f, "reverb: stable on 10 s of full-scale noise");
    }
    // 2. Delay: first echo on L at the delay time, ping-pongs to R at twice that.
    {
        dsp::StereoDelay dl; dl.prepare(kSr);
        const size_t d = static_cast<size_t>(std::floor(0.36 * kSr));
        std::vector<float> L, R;
        for (size_t i = 0; i < d * 3; ++i) { float l, r; dl.process(i == 0 ? 1.f : 0.f, l, r); L.push_back(l); R.push_back(r); }
        size_t firstL = 0; for (size_t i = 0; i < L.size(); ++i) if (std::fabs(L[i]) > 0.5f) { firstL = i; break; }
        size_t pkR = 0; float mR = 0.f;
        for (size_t i = d + d / 2; i < d * 5 / 2 && i < R.size(); ++i) if (std::fabs(R[i]) > mR) { mR = std::fabs(R[i]); pkR = i; }
        std::printf("delay: first L echo at %zu (expect %zu), R echo peak %.3f at %zu (expect ~%zu)\n", firstL, d, mR, pkR, 2 * d);
        CHECK(firstL == d, "delay: first echo at the delay time");
        CHECK(mR > 0.1f && pkR + 4 >= 2 * d && pkR <= 2 * d + 4, "delay: ping-pong echo on R at 2x");
        CHECK(finite(L) && finite(R), "delay: finite");
    }
    // 3. Chorus: finite, level kept, stereo.
    {
        dsp::Chorus ch; ch.prepare(kSr);
        std::vector<float> L, R;
        for (int i = 0; i < static_cast<int>(kSr); ++i) {
            float l, r; ch.process(0.5f * static_cast<float>(std::sin(kTwoPi * 440.0 * i / kSr)), l, r); L.push_back(l); R.push_back(r);
        }
        const size_t s = L.size();
        const double rms = std::sqrt(energy(L, s / 4, s) / (s * 3.0 / 4.0));
        double diff = 0; for (size_t i = s / 4; i < s; ++i) diff += std::fabs(L[i] - R[i]);
        diff /= (s * 3.0 / 4.0);
        std::printf("chorus: rms %.3f (input 0.354), mean |L-R| %.4f\n", rms, diff);
        CHECK(finite(L) && finite(R), "chorus: finite");
        CHECK(rms > 0.25 && rms < 0.45, "chorus: level kept");
        CHECK(diff > 0.005, "chorus: stereo width");
    }
    // 4. Limiter: loud never exceeds ceiling, quiet untouched, gain recovers.
    {
        dsp::Limiter lim; lim.prepare(kSr);
        float pk = 0.f;
        for (int i = 0; i < 48000; ++i) {
            float x = 5.f * static_cast<float>(std::sin(kTwoPi * 220.0 * i / kSr)), y = x;
            lim.process(x, y); pk = std::fmax(pk, std::fmax(std::fabs(x), std::fabs(y)));
        }
        CHECK(pk <= 0.98f + 1e-6f, "limiter: loud signal stays at or below 0.98");
        float maxErr = 0.f;
        for (int i = 0; i < 2 * 48000; ++i) {
            const float in = 0.1f * static_cast<float>(std::sin(kTwoPi * 220.0 * i / kSr));
            float x = in, y = in; lim.process(x, y);
            if (i > 48000) maxErr = std::fmax(maxErr, std::fabs(x - in));
        }
        CHECK(lim.gain() > 0.999f && maxErr < 1e-3f, "limiter: gain recovers, quiet signal untouched");
    }
    // 5. Sends make tails; no send, no tail.
    {
        Rig a, b;
        a.e.setPart(0, sends(0.8f, 0.f, 0.f)); b.e.setPart(0, sends(0.f, 0.f, 0.f));
        a.run(20, {on(60, 100)}); b.run(20, {on(60, 100)});
        a.run(1, {off(60)}); b.run(1, {off(60)});
        a.untilVoicesEnd(); b.untilVoicesEnd();
        a.run(100); b.run(100);
        const float ta = a.peakOver(50), tb = b.peakOver(50);
        std::printf("reverb send tail after voices end: %.5f (send 0.8) vs %.8f (send 0)\n", ta, tb);
        CHECK(a.e.activeVoices() == 0, "sends: voices ended");
        CHECK(ta > 1e-4f, "sends: reverb send leaves an audible tail");
        CHECK(tb < 1e-7f, "sends: no send = silence after release");
    }
    {   // delay echo arrives about 0.36 s later
        Rig a, b;
        a.e.setPart(0, sends(0.f, 1.f, 0.f)); b.e.setPart(0, sends(0.f, 0.f, 0.f));
        a.run(20, {on(60, 100)}); b.run(20, {on(60, 100)});
        a.run(1, {off(60)}); b.run(1, {off(60)});
        a.untilVoicesEnd(); b.untilVoicesEnd();
        const int wait = static_cast<int>(0.28 * kSr / Rig::kBlock);
        a.run(wait); b.run(wait);
        const float ea = a.peakOver(100), eb = b.peakOver(100);  // ~0.28..0.56 s
        CHECK(ea > 1e-3f, "sends: delay echo arrives");
        CHECK(eb < 1e-7f, "sends: no delay send = no echo");
    }
    {   // chorus adds stereo difference to a centre-panned note; Eco switches it off
        Rig a, b, c;
        a.e.setPart(0, sends(0.f, 0.f, 1.f)); b.e.setPart(0, sends(0.f, 0.f, 0.f)); c.e.setPart(0, sends(0.f, 0.f, 1.f));
        c.e.setEco(true);
        a.run(60, {on(60, 100)}); b.run(60, {on(60, 100)}); c.run(60, {on(60, 100)});
        std::printf("chorus stereo diff: %.5f (on) vs %.8f (off) vs %.8f (Eco)\n", a.lastDiff, b.lastDiff, c.lastDiff);
        CHECK(a.lastDiff > 1e-4f, "chorus send: adds stereo movement");
        CHECK(b.lastDiff < 1e-7f, "chorus send 0: centre stays mono");
        CHECK(c.lastDiff < 1e-7f, "Eco: chorus is off");
    }
    // 6. Master volume.
    {
        Rig a, b;
        b.e.setMasterVolume(0.5f);
        a.run(40, {on(69, 100)}); b.run(40, {on(69, 100)});
        const float ratio = b.peak / a.peak;
        CHECK(ratio > 0.45f && ratio < 0.55f, "master volume 0.5 = half level");
    }
    // 7. Panic silences effect tails too.
    {
        Rig a;
        a.e.setPart(0, sends(1.f, 1.f, 1.f));
        a.run(30, {on(60, 120)});
        a.e.panic();
        a.run(1);
        CHECK(a.lastPeak < 1e-7f, "panic: tails are silenced");
    }
    // 8. Everything on at once: 16 Parts, 64 voices, all sends full. Finite, limited, CPU measured.
    {
        for (int ecoPass = 0; ecoPass < 2; ++ecoPass) {
            Rig r;
            for (int p = 0; p < 16; ++p) {
                PartConfig c = sends(1.f, 1.f, 1.f); c.waveform = static_cast<Waveform>(p % 4);
                r.e.setPart(p, c);
            }
            r.e.setEco(ecoPass == 1);
            std::vector<MidiEvent> burst;
            for (int k = 0; k < 8; ++k) burst.push_back(on(static_cast<uint8_t>(40 + k * 4), 127));
            r.run(200, burst);
            CHECK(r.e.activeVoices() == kMaxVoices, "full load: 64 voices");
            CHECK(!r.nan, "full load: no NaN");
            CHECK(r.peak <= 0.98f + 1e-6f, "full load: limiter holds the ceiling");
            const int blocks = 5 * 48000 / Rig::kBlock;
            const auto t0 = std::chrono::steady_clock::now();
            for (int b = 0; b < blocks; ++b) r.e.process(r.L.data(), r.R.data(), Rig::kBlock, nullptr, 0);
            const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
            std::printf("bench%s: 64 voices + reverb + delay + chorus + limiter, 5 s audio in %.3f s = %.1f%% of one core\n",
                        ecoPass ? " (Eco)" : "", secs, 100.0 * secs / 5.0);
        }
    }

    std::printf(fails ? "RESULT: FAIL (%d)\n" : "RESULT: PASS\n", fails);
    return fails ? 1 : 0;
}
