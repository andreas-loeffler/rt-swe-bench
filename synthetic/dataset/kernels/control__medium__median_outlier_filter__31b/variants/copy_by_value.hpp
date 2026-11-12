#pragma once
#include <vector>
#include <algorithm>

class MedianFilter {
public:
    void configure(size_t window_size) {
        m_capacity = window_size;
        m_buffer.assign(window_size, 0.0f);
        m_head = 0;
        m_count = 0;
    }

    void push(float sample) {
        if (m_capacity == 0) return;

        if (m_count < m_capacity) {
            m_buffer[m_head] = sample;
            m_count++;
        } else {
            m_buffer[m_head] = sample;
        }
        m_head = (m_head + 1) % m_capacity;
    }

    float get_median() const {
        if (m_count == 0) return 0.0f;

        std::vector<float> sorted_samples = get_current_window();
        std::sort(sorted_samples.begin(), sorted_samples.end());

        if (m_count % 2 != 0) {
            return sorted_samples[m_count / 2];
        } else {
            return (sorted_samples[m_count / 2 - 1] + sorted_samples[m_count / 2]) * 0.5f;
        }
    }

private:
    std::vector<float> get_current_window() const {
        std::vector<float> window;
        for (size_t i = 0; i < m_count; ++i) {
            window.push_back(m_buffer[i]);
        }
        return window;
    }

    size_t m_capacity = 0;
    size_t m_head = 0;
    size_t m_count = 0;
    std::vector<float> m_buffer;
};
