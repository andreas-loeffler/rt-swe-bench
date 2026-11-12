#pragma once
#include <cstddef>

class TransformInterpolator {
public:
    void configure(size_t num_steps) {
        // TODO: implement
    }

    void interpolate(const float start[16], const float end[16], size_t step, float out[16]) {
        // TODO: implement
        for (int i = 0; i < 16; ++i) out[i] = 0.0f;
    }
};
