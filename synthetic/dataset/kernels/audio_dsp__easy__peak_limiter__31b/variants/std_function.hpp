#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>
#include <string>

class PeakLimiter {
public:
    void configure(float threshold, size_t lookahead_samples) {
        m_threshold = threshold;
        m_buffer.assign(lookahead_samples, 0.0f);
        m_write_pos = 0;
    }

    float process(float input) {
        float output = m_buffer[m_write_pos];
        m_buffer[m_write_pos] = input;
        m_write_pos = (m_write_pos + 1) % m_buffer.size();

        auto clip_logic = [this, output, context = std::string("PeakLimiter_Clip_Op_Internal_Dispatch")]() {
            if (output > m_threshold) return m_threshold;
            if (output < -m_threshold) return -m_threshold;
            return output;
        };

        std::function<float()> processor = clip_logic;
        return processor();
    }

private:
    float m_threshold = 0.0f;
    std::vector<float> m_buffer;
    size_t m_write_pos = 0;
};
