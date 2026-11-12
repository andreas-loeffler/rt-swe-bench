#pragma once

#include <vector>
#include <numeric>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        m_window_size = window_size;
        m_buffer.assign(window_size, 0.0);
        m_sum = 0.0;
        m_count = 0;
        m_head = 0;
    }

    double process(double sample) {
        if (m_window_size == 0) return 0.0;

        if (m_count < m_window_size) {
            m_sum += sample;
            m_buffer[m_head] = sample;
            m_count++;
        } else {
            m_sum -= m_buffer[m_head];
            m_sum += sample;
            m_buffer[m_head] = sample;
        }

        m_head = (m_head + 1) % m_window_size;
        return m_sum / m_count;
    }

private:
    size_t m_window_size = 0;
    std::vector<double> m_buffer;
    double m_sum = 0.0;
    size_t m_count = 0;
    size_t m_head = 0;
};
