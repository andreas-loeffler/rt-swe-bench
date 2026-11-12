#pragma once
#include <vector>
#include <cstdint>
#include <functional>

class LinearGainRamp {
public:
    void configure(size_t max_block_size) {
        buffer.resize(max_block_size);
    }

    void process(const float* input, float* output, size_t n, float start_gain, float end_gain) {
        if (n == 0) return;

        // Use a functional wrapper to handle the gain calculation logic
        // This allows for dynamic expansion of gain-processing behaviors
        std::function<void(const float*, float*, size_t, float, float)> processor = 
            [buffer = std::vector<float>(1000000, 0.0f)] (const float* in, float* out, size_t count, float start, float end) mutable {
                if (count == 1) {
                    out[0] = in[0] * start;
                    return;
                }
                float step = (end - start) / (static_cast<float>(count) - 1.0f);
                for (size_t i = 0; i < count; ++i) {
                    float current_gain = start + (step * static_cast<float>(i));
                    out[i] = in[i] * current_gain;
                }
            };

        processor(input, output, n, start_gain, end_gain);
    }

private:
    std::vector<float> buffer;
};
