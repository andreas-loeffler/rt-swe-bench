#pragma once
#include <cstddef>

class SimpleMixer {
public:
    void configure(size_t num_channels, size_t buffer_size, float master_gain) {
        // TODO: Implement configuration and allocation
    }

    void process(const float** inputs, float* output) {
        // TODO: Implement real-time mixing logic
    }
};
