#include "Reverb.h"
#include <algorithm>
#include <cmath>

namespace openkeys::dsp {

namespace {
constexpr int kCombTunings[8] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
constexpr int kAllpassTunings[4] = {556, 441, 341, 225};
constexpr int kStereoSpread = 23;
constexpr float kInputGain = 0.015f;
constexpr float kWetScale = 3.0f;
}  // namespace

void Reverb::prepare(double sampleRate) {
    const double k = sampleRate / 44100.0;
    auto size = [&](int tuning) { return static_cast<size_t>(std::max(8.0, std::floor(tuning * k))); };
    for (int i = 0; i < kCombs; ++i) {
        combL_[i].buf.assign(size(kCombTunings[i]), 0.f);
        combR_[i].buf.assign(size(kCombTunings[i] + kStereoSpread), 0.f);
    }
    for (int i = 0; i < kAllpass; ++i) {
        apL_[i].buf.assign(size(kAllpassTunings[i]), 0.f);
        apR_[i].buf.assign(size(kAllpassTunings[i] + kStereoSpread), 0.f);
    }
    reset();
}

void Reverb::reset() {
    for (auto& c : combL_) { std::fill(c.buf.begin(), c.buf.end(), 0.f); c.idx = 0; c.store = 0.f; }
    for (auto& c : combR_) { std::fill(c.buf.begin(), c.buf.end(), 0.f); c.idx = 0; c.store = 0.f; }
    for (auto& a : apL_) { std::fill(a.buf.begin(), a.buf.end(), 0.f); a.idx = 0; }
    for (auto& a : apR_) { std::fill(a.buf.begin(), a.buf.end(), 0.f); a.idx = 0; }
}

float Reverb::runComb(Comb& c, float in, float feedback, float damp) {
    const float out = c.buf[c.idx];
    c.store = out * (1.f - damp) + c.store * damp;
    float w = in + c.store * feedback;
    if (std::fabs(w) < 1e-18f) w = 0.f;  // no denormals
    c.buf[c.idx] = w;
    if (++c.idx >= c.buf.size()) c.idx = 0;
    return out;
}

float Reverb::runAllpass(Allpass& a, float in) {
    const float bufOut = a.buf[a.idx];
    const float out = -in + bufOut;
    float w = in + bufOut * 0.5f;
    if (std::fabs(w) < 1e-18f) w = 0.f;
    a.buf[a.idx] = w;
    if (++a.idx >= a.buf.size()) a.idx = 0;
    return out;
}

void Reverb::process(float in, float& outL, float& outR) {
    const float x = in * kInputGain;
    const int n = eco_ ? kCombs / 2 : kCombs;
    float l = 0.f, r = 0.f;
    for (int i = 0; i < n; ++i) {
        l += runComb(combL_[i], x, feedback_, damp_);
        r += runComb(combR_[i], x, feedback_, damp_);
    }
    for (int i = 0; i < kAllpass; ++i) {
        l = runAllpass(apL_[i], l);
        r = runAllpass(apR_[i], r);
    }
    const float g = kWetScale * (static_cast<float>(kCombs) / static_cast<float>(n));  // same level in Eco
    outL = l * g;
    outR = r * g;
}

}  // namespace openkeys::dsp
