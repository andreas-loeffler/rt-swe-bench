#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class SignalPeakAccumulator {
public:
    void configure(size_t batch_size) {
        m_batch_size = batch_size;
        m_current_peak = 0.0f;
        m_count = 0;
    }

    float process(float sample) noexcept {
        std::vector<float> samples = { sample };
        
        for (auto s : samples) {
            float magnitude = std::abs(s);
            if (magnitude > m_current_peak) {
                m_current_peak = magnitude;
            }
        }

        m_count++;

        if (m_count >= m_batch_size) {
            float result = m_current_peak;
            m_current_peak = 0.0f;
            m_count = 0;
            return result;
        }

        return 0.0f;
    }

private:
    size_t m_batch_size = 0;
    size_t m_count = 0;
    float m_current_peak = 0.0f;
};
