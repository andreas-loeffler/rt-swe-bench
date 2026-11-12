#pragma once

#include <vector>
#include <algorithm>

class SortedWindowMedian {
public:
    void configure(size_t window_size) {
        m_size = window_size;
        m_buffer.clear();
        m_sorted.clear();
        m_sorted.reserve(window_size);
        m_buffer.reserve(window_size);
    }

    double push(double sample) {
        if (m_size == 0) return 0.0;

        if (m_buffer.size() < m_size) {
            m_buffer.push_back(sample);
            auto it = std::lower_bound(m_sorted.begin(), m_sorted.end(), sample);
            m_sorted.insert(it, sample);
        } else {
            double oldest = m_buffer[m_head];
            m_buffer[m_head] = sample;
            m_head = (m_head + 1) % m_size;

            auto it = std::lower_bound(m_sorted.begin(), m_sorted.end(), oldest);
            m_sorted.erase(it);
            auto it_new = std::lower_bound(m_sorted.begin(), m_sorted.end(), sample);
            m_sorted.insert(it_new, sample);
        }

        size_t n = m_sorted.size();
        if (n == 0) return 0.0;
        if (n % 2 == 1) {
            return m_sorted[n / 2];
        } else {
            return (m_sorted[n / 2 - 1] + m_sorted[n / 2]) * 0.5;
        }
    }

private:
    size_t m_size = 0;
    size_t m_head = 0;
    std::vector<double> m_buffer;
    std::vector<double> m_sorted;
};
