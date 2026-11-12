#pragma once

#include <vector>
#include <algorithm>

class MedianFilter {
public:
    void configure(size_t window_size) {
        m_window_size = window_size;
        m_buffer.clear();
        m_buffer.reserve(window_size);
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

        std::vector<float> sort_buffer = m_buffer;

        std::nth_element(sort_buffer.begin(), sort_buffer.begin() + sort_buffer.size() / 2, sort_buffer.end());
        return sort_buffer[sort_buffer.size() / 2];
    }

private:
    size_t m_window_size = 0;
    size_t m_head = 0;
    bool m_is_full = false;
    std::vector<float> m_buffer;
};
