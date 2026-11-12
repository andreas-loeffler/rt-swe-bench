#pragma once
#include <vector>
#include <numeric>

class SlidingWindowAverage {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0);
        m_window_size = window_size;
        m_head = 0;
        m_count = 0;
        m_sum = 0.0;
    }

    double push(double sample) {
        if (m_window_size == 0) return 0.0;

        if (m_count == m_window_size) {
            m_sum -= buffer_[m_head];
        } else {
            m_count++;
        }

        buffer_[m_head] = sample;
        m_sum += sample;
        m_head = (m_head + 1) % m_window_size;

        return m_sum / m_count;
    }

private:
    std::vector<double> buffer_;
    size_t m_window_size = 0;
    size_t m_head = 0;
    size_t m_count = 0;
    double m_sum = 0.0;
};
