#pragma once
#include <cstddef>
#include <vector>

class SimpleMixer {
public:
    void configure(size_t num_channels, size_t buffer_size, float master_gain) {
        m_num_channels = num_channels;
        m_buffer_size = buffer_size;
        m_master_gain = master_gain;
    }

    void process(const float** inputs, float* output) {
        // Reset output buffer to zero first
        for (size_t i = 0; i < m_buffer_size; ++i) {
            output[i] = 0.0f;
        }

        // Sum all inputs into output
        for (size_t ch = 0; ch < m_num_channels; ++ch) {
            const float* input_buffer = inputs[ch];
            for (size_t i = 0; i < m_buffer_size; ++i) {
                output[i] += input_buffer[i];
            }
        }

        // Apply master gain
        for (size_t i = 0; i < m_buffer_size; ++i) {
            output[i] *= m_master_gain;
        }
    }

private:
    size_t m_num_channels = 0;
    size_t m_buffer_size = 0;
    float m_master_gain = 1.0f;
};
