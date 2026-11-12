#pragma once
#include <cstddef>
#include <vector>
#include <algorithm>

class TransformInterpolator {
public:
    void configure(size_t num_steps) {
        num_steps_ = num_steps;
        // Reserve memory in configure to satisfy real-time constraints
        // Note: The logic requires num_steps > 1 to avoid division by zero
        // based on the formula (num_steps - 1).
    }

    void interpolate(const float start[16], const float end[16], size_t step, float out[16]) {
        // Formula: out[i] = start[i] + (end[i] - start[i]) * (step / (num_steps - 1))
        // Optimization: Calculate the weight once.
        // If num_steps is 1, the denominator is 0; however, the precondition 
        // implies a range [0, num_steps-1], usually implying num_steps > 1.
        
        float weight = 0.0f;
        if (num_steps_ > 1) {
            weight = static_cast<float>(step) / static_cast<float>(num_steps_ - 1);
        } else {
            weight = 0.0f;
        }

        for (int i = 0; i < 16; ++i) {
            out[i] = start[i] + (end[i] - start[i]) * weight;
        }
    }

private:
    size_t num_steps_ = 0;
};
