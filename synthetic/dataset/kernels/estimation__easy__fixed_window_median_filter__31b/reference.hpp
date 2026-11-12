#pragma once

#include <vector>
#include <algorithm>

class FixedWindowMedianFilter {
public:
    void configure(size_t window_size) {
        m_window_size = window_size;
        m_write_idx = 0;
        m_count = 0;
        m_buffer.assign(window_size, 0.0f);
        m_sort_buffer.assign(window_size, 0.0f);
    }

    float process(float sample) {
        m_buffer[m_write_idx] = sample;
        m_write_idx = (m_write_idx + 1) % m_window_size;
        if (m_count < m_window_size) {
            m_count++;
        }

        // Copy current window to sort buffer
        for (size_t i = 0; i < m_count; ++i) {
            m_sort_buffer[i] = m_buffer[i];
        }

        auto it_mid = m_sort_buffer.begin() + (m_count / 2);
        std::nth_element(m_sort_buffer.begin(), it_mid, m_sort_buffer.begin() + m_count);
        
        if (m_count % 2 != 0) {
            return *it_mid;
        } else {
            float high = *it_mid;
            auto it_low = m_sort_buffer.begin() + (m_count / 2 - 1);
            std::nth_element(m_sort_buffer.begin(), it_low, m_sort_buffer.begin() + m_count);
            float low = *it_low;
            return (low + high) * 0.5f;
        }
    }

private:
    size_t m_window_size = 0;
    size_t m_write_idx = 0;
    size_t m_count = 0;
    std::vector<float> m_buffer;
    std::vector<float> m_sort_buffer;
};
