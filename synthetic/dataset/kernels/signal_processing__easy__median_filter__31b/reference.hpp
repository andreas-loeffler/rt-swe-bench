#pragma once

#include <vector>
#include <algorithm>

class MedianFilter {
public:
    void configure(size_t window_size) {
        m_window_size = window_size;
        m_buffer.clear();
        m_buffer.reserve(window_size);
        m_sort_buffer.resize(window_size);
        m_head = 0;
        m_is_full = false;
    }

    float push(float sample) {
        if (!m_is_full) {
            m_buffer.push_back(sample);
            if (m_buffer.size() == m_window_size) {
                m_is_full = true;
            }
        } else {
            m_buffer[m_head] = sample;
            m_head = (m_head + 1) % m_window_size;
        }

        size_t current_size = m_buffer.size();
        // Use the pre-allocated sort buffer to avoid heap allocation in RT path
        for (size_t i = 0; i < current_size; ++i) {
            m_sort_buffer[i] = m_buffer[i];
        }

        std::nth_element(m_sort_buffer.begin(), m_sort_buffer.begin() + current_size / 2, m_sort_buffer.begin() + current_size);
        return m_sort_buffer[current_size / 2];
    }

private:
    size_t m_window_size = 0;
    size_t m_head = 0;
    bool m_is_full = false;
    std::vector<float> m_buffer;
    std::vector<float> m_sort_buffer;
};
