#pragma once
#include <list>
#include <numeric>
#include <cstddef>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        m_buffer.clear();
        m_window_size = window_size;
        m_sum = 0.0;
    }

    double process(double sample) {
        if (m_window_size == 0) return 0.0;

        m_sum += sample;
        m_buffer.push_back(sample);

        if (m_buffer.size() > m_window_size) {
            m_sum -= m_buffer.front();
            m_buffer.pop_front();
        }

        return m_sum / m_buffer.size();
    }

private:
    std::list<double> m_buffer;
    size_t m_window_size = 0;
    double m_sum = 0.0;
};
