// Freeverb-style stereo reverb (8 combs + 4 allpasses per side). Mono in, stereo out.
// prepare() allocates; process() does not. Eco mode uses 4 combs (about half the CPU).
#pragma once
#include <cstddef>
#include <vector>

namespace openkeys::dsp {

class Reverb {
public:
    void prepare(double sampleRate);
    void reset();
    void setEco(bool eco) { eco_ = eco; }
    void process(float in, float& outL, float& outR);

private:
    static constexpr int kCombs = 8, kAllpass = 4;
    struct Comb { std::vector<float> buf; size_t idx = 0; float store = 0.f; };
    struct Allpass { std::vector<float> buf; size_t idx = 0; };

    static float runComb(Comb& c, float in, float feedback, float damp);
    static float runAllpass(Allpass& a, float in);

    Comb combL_[kCombs], combR_[kCombs];
    Allpass apL_[kAllpass], apR_[kAllpass];
    float feedback_ = 0.86f, damp_ = 0.35f;
    bool eco_ = false;
};

}  // namespace openkeys::dsp
