#pragma once

#include <vector>
#include <numeric>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        m_window_size = window_size;
        m_buffer.clear();
        m_sum = 0.0;
    }

    double process(double sample) {
        if (m_window_size == 0) return 0.0;

        m_buffer.push_back(sample);
        m_sum += sample;

        if (m_buffer.size() > m_window_size) {
            m_sum -= m_buffer.front();
            m_buffer.erase(m_buffer.begin());
        }

        return m_sum / m_buffer.size();
    }

private:
    size_t m_window_size = 0;
    std::vector<double> m_buffer;
    double m_sum = 0.0;
};
