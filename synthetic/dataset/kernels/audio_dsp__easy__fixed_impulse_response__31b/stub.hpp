#pragma once

#include <span>

class ImpulseResponseConvolver {
public:
    void configure(std::span<const float> coefficients) {
        // TODO: Allocate and store coefficients and history buffer
    }

    float process(float input) {
        // TODO: Implement FIR convolution
        return 0.0f;
    }
};
