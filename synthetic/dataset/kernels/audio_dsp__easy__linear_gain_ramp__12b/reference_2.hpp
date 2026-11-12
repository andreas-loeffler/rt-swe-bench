#pragma once
#include <cstddef>
#include <vector>

class LinearGainRamp {
public:
    /**
     * Pre-allocates the internal buffer to ensure no heap allocations occur during process().
     */
    void configure(size_t max_block_size) {
        gain_buffer_.reserve(max_block_size);
    }

    /**
     * Applies a linear gain ramp to the input buffer.
     * Guaranteed to be real-time safe: no allocation, no locking, no I/O.
     */
    void process(const float* input, float* output, size_t n, float start_gain, float end_gain) {
        if (n == 0) {
            return;
        }

        if (n == 1) {
            output[0] = input[0] * start_gain;
            return;
        }

        // Calculate the incremental step per sample
        const float delta = (end_gain - start_gain) / static_cast<float>(n - 1);
        float current_gain = start_gain;

        for (size_t i = 0; i < n; ++i) {
            output[i] = input[i] * current_gain;
            current_gain += delta;
        }

        // Explicitly set the last sample to end_gain to avoid floating point drift
        if (n > 0) {
            output[n - 1] = input[n - 1] * end_gain;
        }
    }

private:
    // Buffer reserved in configure(), though for this specific math implementation 
    // we compute on-the-fly to maximize cache locality and avoid an extra pass.
    std::vector<float> gain_buffer_;
};
