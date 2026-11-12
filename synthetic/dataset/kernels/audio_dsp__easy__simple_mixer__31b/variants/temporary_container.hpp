#pragma once
#include <vector>
#include <cstddef>
#include <numeric>

class SimpleMixer {
public:
    void configure(size_t num_channels, size_t buffer_size, float master_gain) {
        m_num_channels = num_channels;
        m_buffer_size = buffer_size;
        m_master_gain = master_gain;
    }

    void process(const float** inputs, float* output) {
        for (size_t i = 0; i < m_buffer_size; ++i) {
            std::vector<float> samples;
            samples.reserve(m_num_channels);
            for (size_t ch = 0; ch < m_num_channels; ++ch) {
                samples.push_back(inputs[ch][i]);
            }
            
            float sum = std::accumulate(samples.begin(), samples.end(), 0.0f);
            output[i] = sum * m_master_gain;
        }
    }

private:
    size_t m_num_channels = 0;
    size_t m_buffer_size = 0;
    float m_master_gain = 1.0f;
};
