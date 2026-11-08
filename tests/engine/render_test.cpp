// Offline render test: no NaN, no clipping, notes sound and die, 64-voice stress, WAV written.
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>
#include "PartEngine.h"

using namespace openkeys;

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { std::printf("FAIL: %s\n", msg); ++fails; } } while (0)

struct Stats { bool nan = false; float peak = 0.f; };

static void scan(const std::vector<float>& b, Stats& s) {
    for (float x : b) {
        if (!std::isfinite(x)) s.nan = true;
        s.peak = std::fmax(s.peak, std::fabs(x));
    }
}

static void writeWav(const char* path, const std::vector<float>& mono, int sr) {
    FILE* f = std::fopen(path, "wb");
    if (!f) return;
    const uint32_t n = static_cast<uint32_t>(mono.size()), bytes = n * 2;
    auto w32 = [&](uint32_t v) { std::fwrite(&v, 4, 1, f); };
    auto w16 = [&](uint16_t v) { std::fwrite(&v, 2, 1, f); };
    std::fwrite("RIFF", 1, 4, f); w32(36 + bytes); std::fwrite("WAVEfmt ", 1, 8, f);
    w32(16); w16(1); w16(1); w32(sr); w32(sr * 2); w16(2); w16(16);
    std::fwrite("data", 1, 4, f); w32(bytes);
    for (float x : mono) { int16_t s = static_cast<int16_t>(std::lround(x * 32767.f)); w16(static_cast<uint16_t>(s)); }
    std::fclose(f);
}

int main(int argc, char** argv) {
    const char* wavPath = argc > 1 ? argv[1] : "render_test.wav";
    const int sr = 48000, block = 128;
    PartEngine eng;
    eng.prepare(sr, block);

    std::vector<float> L(block), R(block), all;
    Stats st;
    auto run = [&](int blocks, std::vector<MidiEvent> first = {}) {
        for (int b = 0; b < blocks; ++b) {
            eng.process(L.data(), R.data(), block, b == 0 ? first.data() : nullptr,
                        b == 0 ? static_cast<int>(first.size()) : 0);
            scan(L, st); scan(R, st);
            all.insert(all.end(), L.begin(), L.end());
        }
    };

    // 1. Chord C4 E4 G4, then release.
    run(100, {{0, 0x90, 60, 100}, {10, 0x90, 64, 100}, {20, 0x90, 67, 100}});
    CHECK(eng.activeVoices() == 3, "3 voices after chord");
    float energy = 0.f; for (float x : all) energy += x * x;
    CHECK(energy > 1.f, "chord produces sound");
    run(1, {{0, 0x80, 60, 0}, {0, 0x80, 64, 0}, {0, 0x90, 67, 0}});  // vel-0 note-on = off
    run(200);
    CHECK(eng.activeVoices() == 0, "all voices released after note-off");

    // 2. Stress: 64 notes at max velocity (+ 8 extra to force stealing), then panic.
    std::vector<MidiEvent> burst;
    for (int i = 0; i < 72; ++i) burst.push_back({0, 0x90, static_cast<uint8_t>(24 + i), 127});
    run(200, burst);
    CHECK(eng.activeVoices() == kMaxVoices, "voice cap holds at 64 under stealing");
    CHECK(st.peak < 1.0f, "no clipping with 64 loud voices");
    eng.panic();
    run(1);
    CHECK(eng.activeVoices() == 0, "panic silences everything");

    // 3. Benchmark (this machine, NOT the target laptop): 64 voices for 5 s of audio.
    burst.resize(64);
    run(1, burst);
    const int blocks = 5 * sr / block;
    const auto t0 = std::chrono::steady_clock::now();
    for (int b = 0; b < blocks; ++b) eng.process(L.data(), R.data(), block, nullptr, 0);
    const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    std::printf("bench: 64 voices, 5 s audio rendered in %.3f s = %.1f%% of one core\n",
                secs, 100.0 * secs / 5.0);

    CHECK(!st.nan, "no NaN/Inf in output");
    writeWav(wavPath, all, sr);
    std::printf("peak %.3f, %zu samples written to %s\n", st.peak, all.size(), wavPath);
    std::printf(fails ? "RESULT: FAIL (%d)\n" : "RESULT: PASS\n", fails);
    return fails ? 1 : 0;
}
