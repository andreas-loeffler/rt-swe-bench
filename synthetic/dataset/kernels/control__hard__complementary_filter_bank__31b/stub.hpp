#pragma once

#include <cstddef>

class ComplementaryFilterBank {
public:
    void configure(size_t num_axes, double sample_rate, double alpha) {
        // TODO: Implement configuration
    }

    void process(const double* gyro_rates, const double* accels, double* output) {
        // TODO: Implement real-time processing
        for (size_t i = 0; i < 3; ++i) output[i] = 0.0;
    }
};
