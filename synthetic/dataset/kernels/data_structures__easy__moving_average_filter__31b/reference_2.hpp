#pragma once

#include <cstddef>
#include <vector>
#include <numeric>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        m_buffer.resize(window_size);
        m_window_size = window_size;
        m_count = 0;
        m_sum = 0.0;
    }

    double process(double sample) {
        if (m_buffer.empty()) {
            return 0.0;
        }

        // If buffer is full, subtract the oldest sample from the running sum
        if (m_count == m_window_size) {
            m_sum -= m_buffer[m_head];
        } else {
            m_count++;
        }

        // Add new sample to sum and buffer
        m_sum += sample;
        m_buffer[m_head] = sample;
        
        // Move head index (circular buffer logic)
        m_head = (m_head + 1) % m_window_size;

        return m_sum / m_count;
    }

private:
    std::vector<double> m_buffer;
    size_t m_window_size = 0;
    size_t m_count = 0;
    size_t m_head = 0;
    double m_sum = 0.0;
};
