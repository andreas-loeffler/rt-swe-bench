#pragma once
#include <cmath>
#include <algorithm>
#include <memory>

class EnvelopeFollower {
public:
    void configure(float sampleRate, float attackTimeSec, float releaseTimeSec) {
        attackCoef = 1.0f - std::exp(-1.0f / (attackTimeSec * sampleRate));
        releaseCoef = 1.0f - std::exp(-1.0f / (releaseTimeSec * sampleRate));
        envelope = 0.0f;
    }

    float step(float input) noexcept {
        const float absInput = std::abs(input);
        
        auto state = std::make_shared<float>(envelope);

        if (absInput > *state) {
            *state += attackCoef * (absInput - *state);
        } else {
            *state -= releaseCoef * (*state - absInput);
        }
        
        envelope = *state;
        return envelope;
    }

private:
    float attackCoef = 0.0f;
    float releaseCoef = 0.0f;
    float envelope = 0.0f;
};
