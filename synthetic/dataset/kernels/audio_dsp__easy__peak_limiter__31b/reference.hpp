#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class PeakLimiter {
public:
    void configure(float threshold, size_t lookahead_samples) {
        m_threshold = threshold;
        m_buffer.assign(lookahead_samples, 0.0f);
        m_write_pos = 0;
    }

    float process(float input) {
        // Get the sample from the look-ahead delay
        float output = m_buffer[m_write_pos];
        
        // Update buffer with current input
        m_buffer[m_write_pos] = input;
        
        // Advance ring buffer index
        m_write_pos = (m_write_pos + 1) % m_buffer.size();
        
        // Hard clip the delayed sample
        if (output > m_threshold) return m_threshold;
        if (output < -m_threshold) return -m_threshold;
        return output;
    }

private:
    float m_threshold = 0.0f;
    std::vector<float> m_buffer;
    size_t m_write_pos = 0;
};
