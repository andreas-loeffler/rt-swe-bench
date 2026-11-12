#pragma once

#include <cstddef>
#include <vector>
#include <numeric>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        m_window_size = window_size;
        m_buffer.assign(m_window_size, 0.0f);
        m_sum = 0.0f;
        m_count = 0;
        m_write_idx = 0;
    }

    float process(float input) {
        if (m_window_size == 0) return 0.0f;

        if (m_count < m_window_size) {
            // Phase 1: Buffer is not yet full
            m_sum += input;
            m_buffer[m_count] = input;
            m_count++;
            return m_sum / static_cast<float>(m_count);
        } else {
            // Phase 2: Buffer is full, sliding window
            // Subtract the oldest sample and add the new one
            m_sum -= m_buffer[m_write_idx];
            m_buffer[m_write_idx] = input;
            m_sum += input;
            
            m_write_idx = (m_write_idx + 1) % m_window_size;
            return m_sum / static_cast<float>(m_window_size);
        }
    }

private:
    std::vector<float> m_buffer;
    size_t m_window_size = 0;
    size_t m_count = 0;
    size_t m_write_idx = 0;
    float m_sum = 0.0f;
};
