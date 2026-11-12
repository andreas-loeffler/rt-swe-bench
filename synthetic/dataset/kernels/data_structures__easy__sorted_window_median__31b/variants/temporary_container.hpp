#pragma once

#include <vector>
#include <algorithm>

class SortedWindowMedian {
public:
    void configure(size_t window_size) {
        m_size = window_size;
        m_buffer.clear();
        if (m_size > 0) {
            m_buffer.reserve(m_size);
        }
    }

    double push(double sample) {
        if (m_size == 0) return 0.0;

        if (m_buffer.size() < m_size) {
            m_buffer.push_back(sample);
        } else {
            m_buffer[m_head] = sample;
            m_head = (m_head + 1) % m_size;
        }

        std::vector<double> sorted_samples = m_buffer;
        std::sort(sorted_samples.begin(), sorted_samples.end());

        size_t n = sorted_samples.size();
        if (n == 0) return 0.0;
        if (n % 2 == 1) {
            return sorted_samples[n / 2];
        } else {
            return (sorted_samples[n / 2 - 1] + sorted_samples[n / 2]) * 0.5;
        }
    }

private:
    size_t m_size = 0;
    size_t m_head = 0;
    std::vector<double> m_buffer;
};
