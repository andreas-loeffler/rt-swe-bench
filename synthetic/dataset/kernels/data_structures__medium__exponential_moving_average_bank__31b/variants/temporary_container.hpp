#pragma once
#include <vector>
#include <cstddef>
#include <algorithm>

class EMABank {
public:
    void configure(size_t num_channels, const float* coefficients) {
        m_num_channels = num_channels;
        m_alphas.assign(coefficients, coefficients + num_channels);
        m_states.assign(num_channels, 0.0f);
    }

    void process(const float* inputs, float* outputs, size_t num_samples) {
        if (!inputs || !outputs || num_samples == 0) return;

        for (size_t s = 0; s < num_samples; ++s) {
            std::vector<float> current_frame_inputs(m_num_channels);
            for (size_t c = 0; c < m_num_channels; ++c) {
                current_frame_inputs[c] = inputs[s * m_num_channels + c];
            }

            process_frame(current_frame_inputs, outputs, s);
        }
    }

    void reset() {
        std::fill(m_states.begin(), m_states.end(), 0.0f);
    }

    float get_state(size_t channel) const {
        if (channel >= m_num_channels) return 0.0f;
        return m_states[channel];
    }

private:
    void process_frame(const std::vector<float>& frame, float* outputs, size_t sample_index) {
        for (size_t c = 0; c < m_num_channels; ++c) {
            const float x = frame[c];
            const float alpha = m_alphas[c];
            float state = m_states[c];
            
            float y = alpha * x + (1.0f - alpha) * state;
            
            m_states[c] = y;
            outputs[sample_index * m_num_channels + c] = y;
        }
    }

    size_t m_num_channels = 0;
    std::vector<float> m_alphas;
    std::vector<float> m_states;
};
