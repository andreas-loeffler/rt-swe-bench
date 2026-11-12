#pragma once

#include <cstddef>
#include <vector>

class DecimationFilter {
public:
    void configure(size_t decimation_factor) {
        m_factor = decimation_factor;
        m_accumulator.clear();
        m_accumulator.reserve(decimation_factor);
        m_count = 0;
        m_sum = 0.0f;
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
    double m_sum = 0.0;
    std::vector<float> m_accumulator; // Reserved in configure
};
