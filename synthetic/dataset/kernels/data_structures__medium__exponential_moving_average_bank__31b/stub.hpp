#pragma once
#include <cstddef>

class EMABank {
public:
    void configure(size_t num_channels, const float* coefficients) {
        // TODO: Implement allocation and coefficient storage
    }

    void process(const float* inputs, float* outputs, size_t num_samples) {
        // TODO: Implement EMA filtering
    }

    void reset() {
        // TODO: Implement state reset
    }

    float get_state(size_t channel) {
        // TODO: Implement state retrieval
        return 0.0f;
    }
};
