#pragma once

#include <vector>
#include <cstddef>

class DecimationFilter {
public:
    void configure(size_t decimation_factor) {
        m_factor = decimation_factor;
        m_sum = 0.0f;
        m_count = 0;
        // No heap allocation needed for a simple running sum, 
        // but we ensure the state is reset.
    }

    bool push(float sample, float& output) {
        m_sum += sample;
        m_count++;

        if (m_count >= m_factor) {
            output = m_sum / static_cast<float>(m_factor);
            m_sum = 0.0f;
            m_count = 0;
            return true;
        }

        return false;
    }

private:
    size_t m_factor = 0;
    size_t m_count = 0;
    float m_sum = 0.0f;
};
