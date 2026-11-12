#pragma once

#include <vector>
#include <algorithm>

class SortedWindowMedian {
public:
    void configure(size_t window_size) {
        m_size = window_size;
        m_buffer.clear();
        m_sorted.clear();
    }

    double push(double sample) {
        if (m_size == 0) return 0.0;

        if (m_buffer.size() < m_size) {
            m_buffer.push_back(sample);
        } else {
            double oldest = m_buffer[m_head];
            m_buffer[m_head] = sample;
            m_head = (m_head + 1) % m_size;
        }

        std::vector<double> current_window = m_buffer;
        std::sort(current_window.begin(), current_window.end());

        size_t n = current_window.size();
        if (n == 0) return 0.0;
        if (n % 2 == 1) {
            return current_window[n / 2];
        } else {
            return (current_window[n / 2 - 1] + current_window[n / 2]) * 0.5;
        }
    }

private:
    size_t m_size = 0;
    size_t m_head = 0;
    std::vector<double> m_buffer;
    std::vector<double> m_sorted;
};
