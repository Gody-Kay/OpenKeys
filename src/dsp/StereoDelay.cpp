#include "StereoDelay.h"
#include <algorithm>
#include <cmath>

namespace openkeys::dsp {

void StereoDelay::prepare(double sampleRate, double maxSeconds) {
    sampleRate_ = sampleRate;
    const size_t n = static_cast<size_t>(std::ceil(sampleRate * maxSeconds)) + 2;
    bufL_.assign(n, 0.f);
    bufR_.assign(n, 0.f);
    setTime(0.36);
    reset();
}

void StereoDelay::reset() {
    std::fill(bufL_.begin(), bufL_.end(), 0.f);
    std::fill(bufR_.begin(), bufR_.end(), 0.f);
    write_ = 0;
    lpL_ = lpR_ = 0.f;
}

void StereoDelay::setTime(double seconds) {
    const size_t maxD = bufL_.empty() ? 1 : bufL_.size() - 1;
    const double d = std::floor(seconds * sampleRate_);
    delay_ = static_cast<size_t>(std::max(1.0, std::min(d, static_cast<double>(maxD))));
}

void StereoDelay::process(float in, float& outL, float& outR) {
    const size_t n = bufL_.size();
    const size_t read = (write_ + n - delay_) % n;
    const float yL = bufL_[read], yR = bufR_[read];
    lpL_ += kDamp * (yL - lpL_);
    lpR_ += kDamp * (yR - lpR_);
    float wl = in + feedback_ * lpR_;   // L echo feeds R, R echo feeds L: ping-pong
    float wr = feedback_ * lpL_;
    if (std::fabs(wl) < 1e-18f) wl = 0.f;
    if (std::fabs(wr) < 1e-18f) wr = 0.f;
    bufL_[write_] = wl;
    bufR_[write_] = wr;
    if (++write_ >= n) write_ = 0;
    outL = yL;
    outR = yR;
}

}  // namespace openkeys::dsp
