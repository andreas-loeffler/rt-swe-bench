#pragma once

#include <vector>

enum class WeightMode {
    Linear,
    Gaussian
};

class WeightedMovingAverage {
public:
    void configure(size_t window_size, WeightMode mode, float sigma = 1.0f) {
        // TODO: Implement allocation and weight generation
    }

    void push(float sample) {
        // TODO: Implement circular buffer push
    }

    float process() {
        // TODO: Implement weighted average calculation
        return 0.0f;
    }

    void reset() {
        // TODO: Implement buffer reset
    }
};
