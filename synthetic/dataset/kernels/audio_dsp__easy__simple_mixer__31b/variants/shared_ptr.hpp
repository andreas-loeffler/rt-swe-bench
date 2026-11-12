#pragma once
#include <vector>
#include <memory>
#include <cstddef>

class SimpleMixer {
public:
    void configure(size_t num_channels, size_t buffer_size, float master_gain) {
        m_num_channels = num_channels;
        m_buffer_size = buffer_size;
        m_master_gain = master_gain;
    }

    void process(const float** inputs, float* output) {
        auto mix_state = std::make_unique<MixState>();

        for (size_t i = 0; i < m_buffer_size; ++i) {
            mix_state->sum = 0.0f;
            for (size_t ch = 0; ch < m_num_channels; ++ch) {
                mix_state->sum += inputs[ch][i];
            }
            output[i] = mix_state->sum * m_master_gain;
        }
    }

private:
    struct MixState {
        float sum = 0.0f;
    };

    size_t m_num_channels = 0;
    size_t m_buffer_size = 0;
    float m_master_gain = 1.0f;
};
