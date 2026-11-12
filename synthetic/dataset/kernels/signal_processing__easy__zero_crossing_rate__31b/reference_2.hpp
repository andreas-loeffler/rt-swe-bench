#pragma once

#include <vector>
#include <cstddef>

class ZeroCrossingRate {
public:
    /**
     * Initializes the internal buffer to hold window_size samples.
     * Precondition: window_size > 1.
     */
    void configure(size_t window_size) {
        m_window_size = window_size;
        m_buffer.assign(m_window_size, 0.0f);
        m_write_idx = 0;
        m_count = 0;
        m_crossings = 0;
    }

    /**
     * Pushes a new sample into the sliding window and returns the current ZCR.
     * ZCR = (crossings in window) / (samples_in_window - 1).
     */
    float push(float sample) {
        // Handle cases where push is called before configure or with invalid window size
        if (m_window_size <= 1 || m_buffer.empty()) {
            return 0.0f;
        }

        // 1. If the buffer is full, the sample at m_write_idx is about to be overwritten.
        // We must remove the crossing contribution between the sample being replaced 
        // and the one immediately following it.
        if (m_count == m_window_size) {
            float first = m_buffer[m_write_idx];
            float second = m_buffer[(m_write_idx + 1) % m_window_size];

            bool first_pos = first > 0.0f;
            bool first_neg = first < 0.0f;
            bool second_pos = second > 0.0f;
            bool second_neg = second < 0.0f;

            if ((first_pos && second_neg) || (first_neg && second_pos)) {
                m_crossings--;
            }
        }

        // 2. If there is a previous sample, check if a new crossing occurs.
        if (m_count > 0) {
            // The last sample added is at (m_write_idx - 1 + m_window_size) % m_window_size
            size_t last_idx = (m_write_idx + m_window_size - 1) % m_window_size;
            float prev_sample = m_buffer[last_idx];

            bool prev_pos = prev_sample > 0.0f;
            bool prev_neg = prev_sample < 0.0f;
            bool curr_pos = sample > 0.0f;
            bool curr_neg = sample < 0.0f;

            if ((prev_pos && curr_neg) || (prev_neg && curr_pos)) {
                m_crossings++;
            }
        }

        // 3. Update circular buffer
        m_buffer[m_write_idx] = sample;
        m_write_idx = (m_write_idx + 1) % m_window_size;
        
        if (m_count < m_window_size) {
            m_count++;
        }

        // 4. Calculate and return ZCR
        if (m_count <= 1) {
            return 0.0f;
        }
        return static_cast<float>(m_crossings) / static_cast<float>(m_count - 1);
    }

private:
    std::vector<float> m_buffer;
    size_t m_window_size = 0;
    size_t m_write_idx = 0;
    size_t m_count = 0;
    int m_crossings = 0;
};
