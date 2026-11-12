#pragma once

#include <deque>
#include <cstddef>
#include <numeric>

class SlidingWindowAverage {
public:
    void configure(size_t window_size) {
        m_buffer.clear();
        m_window_size = window_size;
    }

    void push(double sample) noexcept {
        if (m_window_size == 0) return;

        m_buffer.push_back(sample);
        m_sum += sample;

        if (m_buffer.size() > m_window_size) {
            m_sum -= m_buffer.front();
            m_buffer.pop_front();
        }
    }

    double get_average() const noexcept {
        if (m_buffer.empty()) {
            return 0.0;
        }
        return m_sum / static_cast<double>(m_buffer.size());
    }

private:
    std::deque<double> m_buffer;
    size_t m_window_size = 0;
    double m_sum = 0.0;
};
