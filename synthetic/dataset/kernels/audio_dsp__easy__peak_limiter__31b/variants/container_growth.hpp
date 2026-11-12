#pragma once
#include <deque>
#include <cmath>
#include <algorithm>

class PeakLimiter {
public:
    void configure(float threshold, size_t lookahead_samples) {
        m_threshold = threshold;
        m_buffer.clear();
        for (size_t i = 0; i < lookahead_samples; ++i) {
            m_buffer.push_back(0.0f);
        }
    }

    float process(float input) {
        float output = m_buffer.front();
        m_buffer.pop_front();
        m_buffer.push_back(input);

        if (output > m_threshold) return m_threshold;
        if (output < -m_threshold) return -m_threshold;
        return output;
    }

private:
    float m_threshold = 0.0f;
    std::deque<float> m_buffer;
};
