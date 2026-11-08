// Brickwall peak limiter (no lookahead): instant attack, smooth release. Output never exceeds the ceiling.
// Linked stereo so the image does not shift. Header-only, no allocation.
#pragma once
#include <algorithm>
#include <cmath>

namespace openkeys::dsp {

class Limiter {
public:
    void prepare(double sampleRate, float ceiling = 0.98f, double releaseSec = 0.12) {
        ceiling_ = ceiling;
        releaseCoef_ = static_cast<float>(1.0 - std::exp(-1.0 / (releaseSec * sampleRate)));
        gain_ = 1.f;
    }
    void reset() { gain_ = 1.f; }
    float gain() const { return gain_; }

    void process(float& l, float& r) {
        const float peak = std::max(std::fabs(l), std::fabs(r));
        const float target = peak > ceiling_ ? ceiling_ / peak : 1.f;
        if (target < gain_) gain_ = target;                    // instant attack
        else gain_ = std::min(target, gain_ + (1.f - gain_) * releaseCoef_);  // smooth release, never above what this sample allows
        l *= gain_;
        r *= gain_;
    }

private:
    float ceiling_ = 0.98f, releaseCoef_ = 0.001f, gain_ = 1.f;
};

}  // namespace openkeys::dsp
