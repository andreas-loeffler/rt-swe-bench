#pragma once
#include <cstddef>
#include <vector>
#include <algorithm>

class MedianFilter {
public:
    void configure(size_t window_size) {
        m_window_size = window_size;
        m_buffer.resize(m_window_size);
        m_sorted_buffer.resize(m_window_size);
        m_write_idx = 0;
        m_count = 0;
    }

    void push(float sample) {
        m_buffer[m_write_idx] = sample;
        m_write_idx = (m_write_idx + 1) % m_window_size;
        if (m_count < m_window_size) {
            m_count++;
        }
    }

    float process() {
        if (m_count == 0) {
            return 0.0f;
        }

        // Copy valid samples to a contiguous buffer for sorting
        // Note: Since we are in a real-time path, we avoid std::vector allocations.
        // We use the pre-allocated m_sorted_buffer.
        for (size_t i = 0; i < m_count; ++i) {
            // Calculate index in circular buffer
            // If count < window_size, the oldest are at the start of m_buffer
            // If count == window_size, the oldest is at m_write_idx
            size_t idx;
            if (m_count < m_window_size) {
                idx = i;
            } else {
                idx = (m_write_idx + i) % m_window_size;
            }
            m_sorted_buffer[i] = m_buffer[idx];
        }

        // Use std::nth_element or std::sort. std::sort is O(N log N), 
        // nth_element is O(N). Both are allowed as long as no allocation occurs.
        std::nth_element(m_sorted_buffer.begin(), 
                         m_sorted_buffer.begin() + (m_count - 1) / 2, 
                         m_sorted_buffer.begin() + m_count);

        float median;
        if (m_count % 2 == 1) {
            median = m_sorted_buffer[m_count / 2];
        } else {
            float m1 = m_sorted_buffer[m_count / 2 - 1];
            float m2 = m_sorted_buffer[m_count / 2];
            median = (m1 + m2) * 0.5f;
            
            // Re-run nth_element for the second middle element if it moved
            // because nth_element only guarantees the element at the iterator position.
            std::nth_element(m_sorted_buffer.begin(), 
                             m_sorted_buffer.begin() + m_count / 2, 
                             m_sorted_buffer.begin() + m_count);
            m2 = m_sorted_buffer[m_count / 2];
            median = (m1 + m2) * 0.5f;
        }

        return median;
    }

private:
    size_t m_window_size = 0;
    size_t m_write_idx = 0;
    size_t m_count = 0;
    std::vector<float> m_buffer;
    std::vector<float> m_sorted_buffer;
};
