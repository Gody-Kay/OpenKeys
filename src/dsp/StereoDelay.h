// Ping-pong stereo delay with a darkening filter in the feedback path. Mono in, stereo out.
#pragma once
#include <cstddef>
#include <vector>

namespace openkeys::dsp {

class StereoDelay {
public:
    void prepare(double sampleRate, double maxSeconds = 2.0);
    void reset();
    void setTime(double seconds);          // clamped to the prepared maximum
    void setFeedback(float fb) { feedback_ = fb < 0.f ? 0.f : (fb > 0.9f ? 0.9f : fb); }
    void process(float in, float& outL, float& outR);

private:
    std::vector<float> bufL_, bufR_;
    size_t write_ = 0, delay_ = 1;
    double sampleRate_ = 48000.0;
    float feedback_ = 0.38f;
    float lpL_ = 0.f, lpR_ = 0.f;
    static constexpr float kDamp = 0.55f;  // 1 = no filtering
};

}  // namespace openkeys::dsp
