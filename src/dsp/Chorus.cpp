#include "Chorus.h"
#include <algorithm>
#include <cmath>

namespace openkeys::dsp {

namespace {
constexpr double kTwoPi = 6.283185307179586476925286766559;
constexpr double kBaseSec = 0.012;   // 12 ms centre delay
constexpr double kDepthSec = 0.003;  // +-3 ms sweep
}  // namespace

void Chorus::prepare(double sampleRate) {
    sampleRate_ = sampleRate;
    buf_.assign(static_cast<size_t>(sampleRate * 0.05) + 4, 0.f);  // 50 ms
    incL_ = 0.35 / sampleRate;
    incR_ = 0.52 / sampleRate;
    reset();
}

void Chorus::reset() {
    std::fill(buf_.begin(), buf_.end(), 0.f);
    write_ = 0;
    phaseL_ = 0.0;
    phaseR_ = 0.25;
}

float Chorus::tap(double delaySamples) const {
    const size_t n = buf_.size();
    double pos = static_cast<double>(write_) - delaySamples;
    while (pos < 0.0) pos += static_cast<double>(n);
    const size_t i0 = static_cast<size_t>(pos) % n;
    const size_t i1 = (i0 + 1) % n;
    const float frac = static_cast<float>(pos - std::floor(pos));
    return buf_[i0] * (1.f - frac) + buf_[i1] * frac;
}

void Chorus::process(float in, float& outL, float& outR) {
    buf_[write_] = in;
    const double dl = (kBaseSec + kDepthSec * std::sin(phaseL_ * kTwoPi)) * sampleRate_;
    const double dr = (kBaseSec + kDepthSec * std::sin(phaseR_ * kTwoPi)) * sampleRate_;
    outL = tap(dl);
    outR = tap(dr);
    phaseL_ += incL_; if (phaseL_ >= 1.0) phaseL_ -= 1.0;
    phaseR_ += incR_; if (phaseR_ >= 1.0) phaseR_ -= 1.0;
    if (++write_ >= buf_.size()) write_ = 0;
}

}  // namespace openkeys::dsp
