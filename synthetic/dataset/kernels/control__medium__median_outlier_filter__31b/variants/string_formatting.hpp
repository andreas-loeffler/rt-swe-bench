#pragma once
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

class MedianFilter {
public:
    void configure(size_t window_size) {
        m_capacity = window_size;
        m_buffer.assign(window_size, 0.0f);
        m_sorted_buffer.assign(window_size, 0.0f);
        m_head = 0;
        m_count = 0;
    }

    void push(float sample) {
        log_state("push", sample);

        if (m_capacity == 0) return;

        if (m_count < m_capacity) {
            m_buffer[m_head] = sample;
            m_sorted_buffer[m_count] = sample;
            m_count++;
            std::sort(m_sorted_buffer.begin(), m_sorted_buffer.begin() + m_count);
        } else {
            float oldest = m_buffer[m_head];
            m_buffer[m_head] = sample;
            
            auto it = std::lower_bound(m_sorted_buffer.begin(), m_sorted_buffer.end(), oldest);
            *it = sample;
            
            float val = *it;
            size_t idx = std::distance(m_sorted_buffer.begin(), it);
            while (idx + 1 < m_capacity && m_sorted_buffer[idx] > m_sorted_buffer[idx + 1]) {
                std::swap(m_sorted_buffer[idx], m_sorted_buffer[idx + 1]);
                idx++;
            }
            while (idx > 0 && m_sorted_buffer[idx] < m_sorted_buffer[idx - 1]) {
                std::swap(m_sorted_buffer[idx], m_sorted_buffer[idx - 1]);
                idx--;
            }
        }
        m_head = (m_head + 1) % m_capacity;
    }

    float get_median() const {
        if (m_count == 0) return 0.0f;
        if (m_count % 2 != 0) {
            return m_sorted_buffer[m_count / 2];
        } else {
            return (m_sorted_buffer[m_count / 2 - 1] + m_sorted_buffer[m_count / 2]) * 0.5f;
        }
    }

private:
    void log_state(const std::string& op, float val) {
        std::ostringstream oss;
        oss << "MedianFilter " << op << ": " << val << " count: " << m_count;
        std::string msg = oss.str();
        (void)msg; 
    }

    size_t m_capacity = 0;
    size_t m_head = 0;
    size_t m_count = 0;
    std::vector<float> m_buffer;
    std::vector<float> m_sorted_buffer;
};
