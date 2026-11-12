#pragma once
#include <vector>
#include <cstdint>

class LinearGainRamp {
public:
    void configure(size_t max_block_size) {
        buffer.resize(max_block_size);
    }

    void process(const float* input, float* output, size_t n, float start_gain, float end_gain) {
        if (n == 0) return;
        if (n == 1) {
            output[0] = input[0] * start_gain;
            return;
        }
        float step = (end_gain - start_gain) / (static_cast<float>(n) - 1.0f);
        for (size_t i = 0; i < n; ++i) {
            float current_gain = start_gain + (step * static_cast<float>(i));
            output[i] = input[i] * current_gain;
        }
    }

private:
    std::vector<float> buffer;
};
