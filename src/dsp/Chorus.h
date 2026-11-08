// Stereo chorus: two slowly modulated short delays. Mono in, wet-only stereo out.
#pragma once
#include <cstddef>
#include <vector>

namespace openkeys::dsp {

class Chorus {
public:
    void prepare(double sampleRate);
    void reset();
    void process(float in, float& outL, float& outR);

private:
    float tap(double delaySamples) const;

    std::vector<float> buf_;
    size_t write_ = 0;
    double sampleRate_ = 48000.0;
    double phaseL_ = 0.0, phaseR_ = 0.25;
    double incL_ = 0.0, incR_ = 0.0;
};

}  // namespace openkeys::dsp
