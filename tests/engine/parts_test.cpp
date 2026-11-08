// Phase 1 tests: splits, layers, channel/velocity zones, velocity curves, pedal + half-pedal,
// seamless switching, voice cap, panic, pan, benchmark. Offline, no audio device.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <vector>
#include "PartEngine.h"

using namespace openkeys;

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { std::printf("FAIL: %s\n", msg); ++fails; } } while (0)

static MidiEvent on(uint8_t ch, uint8_t key, uint8_t vel) { return {0, static_cast<uint8_t>(0x90 | (ch - 1)), key, vel}; }
static MidiEvent off(uint8_t ch, uint8_t key) { return {0, static_cast<uint8_t>(0x80 | (ch - 1)), key, 0}; }
static MidiEvent cc(uint8_t ch, uint8_t num, uint8_t val) { return {0, static_cast<uint8_t>(0xB0 | (ch - 1)), num, val}; }

struct Rig {
    static constexpr int kBlock = 128;
    PartEngine e;
    std::vector<float> L = std::vector<float>(kBlock), R = std::vector<float>(kBlock);
    std::vector<float> allL, allR;
    bool nan = false;
    float peak = 0.f;

    Rig() { e.prepare(48000.0, kBlock); }

    void run(int blocks, std::vector<MidiEvent> first = {}) {
        for (int b = 0; b < blocks; ++b) {
            e.process(L.data(), R.data(), kBlock, b == 0 ? first.data() : nullptr,
                      b == 0 ? static_cast<int>(first.size()) : 0);
            for (int i = 0; i < kBlock; ++i) {
                if (!std::isfinite(L[i]) || !std::isfinite(R[i])) nan = true;
                peak = std::fmax(peak, std::fmax(std::fabs(L[i]), std::fabs(R[i])));
            }
            allL.insert(allL.end(), L.begin(), L.end());
            allR.insert(allR.end(), R.begin(), R.end());
        }
    }
    int blocksUntilSilent(int maxBlocks) {
        for (int b = 0; b < maxBlocks; ++b) { if (e.activeVoices() == 0) return b; run(1); }
        return maxBlocks;
    }
    float lastPeak() const {
        float p = 0.f;
        for (size_t i = allL.size() - kBlock; i < allL.size(); ++i) p = std::fmax(p, std::fabs(allL[i]));
        return p;
    }
};

static PartConfig zone(uint8_t lo, uint8_t hi) { PartConfig c; c.keyLow = lo; c.keyHigh = hi; return c; }

int main() {
    // 1. Split: keys 0-59 -> Part 1, keys 60-127 -> Part 2.
    {
        Rig r;
        r.e.setPart(0, zone(0, 59));
        r.e.setPart(1, zone(60, 127));
        r.run(1, {on(1, 48, 100)});
        CHECK(r.e.partVoices(0) == 1 && r.e.partVoices(1) == 0, "split: low note only in Part 1");
        r.run(1, {on(1, 72, 100)});
        CHECK(r.e.partVoices(0) == 1 && r.e.partVoices(1) == 1, "split: high note only in Part 2");
    }
    // 2. Layer: two full-range Parts -> one key makes two voices.
    {
        Rig r;
        r.e.setPart(1, PartConfig{});
        r.run(1, {on(1, 60, 100)});
        CHECK(r.e.activeVoices() == 2, "layer: one key = two voices");
    }
    // 3. Channel and velocity zones.
    {
        Rig r;
        PartConfig c; c.channel = 2; c.velLow = 100;
        r.e.setPart(0, c);
        r.run(1, {on(1, 60, 120)});
        CHECK(r.e.activeVoices() == 0, "zone: wrong channel ignored");
        r.run(1, {on(2, 60, 50)});
        CHECK(r.e.activeVoices() == 0, "zone: too soft ignored");
        r.run(1, {on(2, 60, 120)});
        CHECK(r.e.activeVoices() == 1, "zone: right channel + velocity plays");
    }
    // 4. Velocity curves: soft is louder than hard at the same velocity.
    {
        Rig soft, hard;
        PartConfig a, b; a.velocityCurve = VelocityCurve::Soft; b.velocityCurve = VelocityCurve::Hard;
        soft.e.setPart(0, a); hard.e.setPart(0, b);
        soft.run(40, {on(1, 69, 64)}); hard.run(40, {on(1, 69, 64)});
        CHECK(soft.peak > hard.peak * 1.5f, "velocity curve: soft louder than hard at vel 64");
    }
    // 5. Sustain pedal holds, release lets go.
    {
        Rig r;
        r.run(1, {on(1, 60, 100), cc(1, 64, 127)});
        r.run(10);
        r.run(1, {off(1, 60)});
        r.run(200);  // ~0.5 s
        CHECK(r.e.activeVoices() == 1, "sustain: note still held after key-up");
        CHECK(r.lastPeak() > 0.01f, "sustain: still audible");
        r.run(1, {cc(1, 64, 0)});
        CHECK(r.blocksUntilSilent(400) < 400, "sustain: pedal up releases the note");
    }
    // 6. Half-pedal: light pedal makes the tail clearly longer than no pedal.
    {
        Rig none, half;
        none.run(10, {on(1, 60, 100)});
        half.run(1, {cc(1, 64, 38)});
        half.run(10, {on(1, 60, 100)});
        none.run(1, {off(1, 60)});
        half.run(1, {off(1, 60)});
        const int tNone = none.blocksUntilSilent(1000);
        const int tHalf = half.blocksUntilSilent(1000);
        std::printf("half-pedal: tail %d blocks (no pedal) vs %d blocks (30%% pedal)\n", tNone, tHalf);
        CHECK(tHalf >= tNone * 2 && tHalf < 1000, "half-pedal: tail at least 2x longer");
    }
    // 7. Seamless switching: a held note ignores a later setPart (pan hard left, new sound).
    {
        Rig r;
        r.run(1, {on(1, 69, 127)});
        r.run(50);
        const size_t swapAt = r.allL.size();
        PartConfig c; c.pan = -1.f; c.waveform = Waveform::Saw; c.releaseSec = 1.5f;
        r.e.setPart(0, c);
        r.run(50);
        CHECK(r.e.activeVoices() == 1, "switch: held note still sounding");
        float maxStep = 0.f, pR = 0.f, pL = 0.f;
        for (size_t i = swapAt - 20; i < r.allL.size(); ++i) maxStep = std::fmax(maxStep, std::fabs(r.allL[i] - r.allL[i - 1]));
        for (size_t i = swapAt + 128; i < r.allL.size(); ++i) { pR = std::fmax(pR, std::fabs(r.allR[i])); pL = std::fmax(pL, std::fabs(r.allL[i])); }
        std::printf("switch: max sample step across swap %.5f, R/L peak after swap %.3f / %.3f\n", maxStep, pR, pL);
        CHECK(maxStep < 0.01f, "switch: no click across the swap");
        CHECK(pR > 0.8f * pL, "switch: held note keeps its old (centre) pan");
        r.run(1, {on(1, 72, 100)});
        CHECK(r.e.activeVoices() == 2, "switch: new note uses new settings alongside the held one");
    }
    // 8. Pan: hard left leaves the right channel silent.
    {
        Rig r;
        PartConfig c; c.pan = -1.f;
        r.e.setPart(0, c);
        r.run(20, {on(1, 60, 100)});
        float pR = 0.f; for (float x : r.allR) pR = std::fmax(pR, std::fabs(x));
        CHECK(pR < 1e-6f && r.peak > 0.01f, "pan: hard left = right silent");
    }
    // 9. 16 layered Parts, 8 loud notes: cap holds, no NaN, no clipping, panic works.
    {
        Rig r;
        for (int p = 0; p < 16; ++p) { PartConfig c; c.waveform = static_cast<Waveform>(p % 4); r.e.setPart(p, c); }
        std::vector<MidiEvent> burst;
        for (int k = 0; k < 8; ++k) burst.push_back(on(1, static_cast<uint8_t>(40 + k * 4), 127));
        r.run(100, burst);
        int sum = 0; for (int p = 0; p < 16; ++p) sum += r.e.partVoices(p);
        CHECK(r.e.activeVoices() == kMaxVoices, "cap: exactly 64 voices under 128 requests");
        CHECK(sum == kMaxVoices, "cap: per-Part counts add up");
        CHECK(!r.nan && r.peak < 1.f, "cap: no NaN, no clipping");
        r.e.panic();
        r.run(1);
        CHECK(r.e.activeVoices() == 0, "panic silences everything");
    }
    // 10. Benchmark (this machine): 16 Parts, 64 saw/square/tri/sine voices, 5 s.
    {
        Rig r;
        for (int p = 0; p < 16; ++p) { PartConfig c; c.waveform = static_cast<Waveform>(p % 4); r.e.setPart(p, c); }
        std::vector<MidiEvent> burst;
        for (int k = 0; k < 4; ++k) burst.push_back(on(1, static_cast<uint8_t>(48 + k * 5), 100));
        r.run(1, burst);
        const int blocks = 5 * 48000 / Rig::kBlock;
        const auto t0 = std::chrono::steady_clock::now();
        for (int b = 0; b < blocks; ++b) r.e.process(r.L.data(), r.R.data(), Rig::kBlock, nullptr, 0);
        const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        std::printf("bench: %d voices (16 layered Parts), 5 s audio in %.3f s = %.1f%% of one core\n",
                    r.e.activeVoices(), secs, 100.0 * secs / 5.0);
    }

    std::printf(fails ? "RESULT: FAIL (%d)\n" : "RESULT: PASS\n", fails);
    return fails ? 1 : 0;
}
