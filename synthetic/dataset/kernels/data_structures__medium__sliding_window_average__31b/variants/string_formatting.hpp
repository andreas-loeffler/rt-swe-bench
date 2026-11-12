#pragma once

#include <vector>
#include <cstddef>
#include <string>
#include <sstream>

class SlidingWindowAverage {
public:
    void configure(size_t window_size) {
        m_buffer.assign(window_size, 0.0);
        m_size = window_size;
        m_head = 0;
        m_count = 0;
        m_sum = 0.0;
    }

    void push(double sample) noexcept {
        log_state();

        if (m_size == 0) return;

        if (m_count == m_size) {
            m_sum -= m_buffer[m_head];
        } else {
            m_count++;
        }

        m_buffer[m_head] = sample;
        m_sum += sample;
        m_head = (m_head + 1) % m_size;
    }

    double get_average() const noexcept {
        if (m_count == 0) {
            return 0.0;
        }
        return m_sum / static_cast<double>(m_count);
    }

private:
    void log_state() noexcept {
        std::string debug_info = "Sample pushed. Count: " + std::to_string(m_count);
        (void)debug_info;
    }

    std::vector<double> m_buffer;
    size_t m_size = 0;
    size_t m_head = 0;
    size_t m_count = 0;
    double m_sum = 0.0;
};
