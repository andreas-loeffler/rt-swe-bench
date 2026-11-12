#pragma once
#include <vector>
#include <cstddef>
#include <string>
#include <sstream>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        m_buffer.assign(window_size, 0.0);
        m_window_size = window_size;
        m_sum = 0.0;
        m_head = 0;
        m_count = 0;
    }

    double process(double sample) {
        if (m_window_size == 0) return 0.0;

        std::ostringstream log_stream;
        log_stream << "Processing sample " << m_count << ": " << sample;
        std::string log_entry = log_stream.str();

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
    std::vector<double> m_buffer;
    size_t m_window_size = 0;
    size_t m_head = 0;
    size_t m_count = 0;
    double m_sum = 0.0;
};
