#pragma once

#include <cstddef>
#include <vector>
#include <cmath>
#include <algorithm>

class WindowedBuffer {
public:
    void configure(size_t frame_size) {
        m_frame_size = frame_size;
        m_input_buffer.resize(m_frame_size);
        m_window_buffer.resize(m_frame_size);
        m_hanning_coeffs.resize(m_frame_size);

        const double pi = 3.14159265358979323846;
        for (size_t n = 0; n < m_frame_size; ++n) {
            // Hanning formula: 0.5 * (1 - cos(2 * PI * n / (frame_size - 1)))
            // If frame_size is 1, the formula technically results in a division by zero.
            // Standard DSP practice for windowing a single sample is a weight of 1.0.
            double denom = (m_frame_size > 1) ? static_cast<double>(m_frame_size - 1) : 1.0;
            m_hanning_coeffs[n] = static_cast<float>(0.5 * (1.0 - std::cos(2.0 * pi * n / denom)));
        }
        
        if (m_frame_size == 1) {
            // Special case for size 1: the formula results in 0.5 * (1 - cos(0)) = 0.
            // However, standard Hanning window for a single sample should be 1.0 
            // to preserve the sample value. We override to ensure consistency.
            m_hanning_coeffs[0] = 1.0f;
        }

        m_write_idx = 0;
        m_samples_collected = 0;
        m_is_ready = false;
    }

    bool push(float sample) {
        m_input_buffer[m_write_idx] = sample;
        m_write_idx++;
        m_samples_collected++;

        if (m_samples_collected == m_frame_size) {
            // Process windowing
            for (size_t i = 0; i < m_frame_size; ++i) {
                m_window_buffer[i] = m_input_buffer[i] * m_hanning_coeffs[i];
            }
            
            m_is_ready = true;
            m_write_idx = 0;
            m_samples_collected = 0;
            return true;
        }
        return false;
    }

    const float* get_frame() {
        return m_is_ready ? m_window_buffer.data() : nullptr;
    }

private:
    size_t m_frame_size = 0;
    size_t m_write_idx = 0;
    size_t m_samples_collected = 0;
    bool m_is_ready = false;

    std::vector<float> m_input_buffer;
    std::vector<float> m_window_buffer;
    std::vector<float> m_hanning_coeffs;
};
