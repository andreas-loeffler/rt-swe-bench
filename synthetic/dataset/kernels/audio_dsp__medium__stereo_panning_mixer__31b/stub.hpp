#pragma once
#include <cstddef>

class StereoMixer {
public:
    void configure(size_t max_channels, size_t block_size) {
        // TODO: allocate internal buffers
    }

    void set_pan(size_t channel_idx, float pan) {
        // TODO: calculate and store gains
    }

    void process(const float* const* input_buffers, size_t num_channels, float* left_out, float* right_out, size_t num_samples) {
        // TODO: mix mono inputs to stereo output
    }
};
