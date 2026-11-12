#pragma once

#include <vector>
#include <cstddef>
#include <numeric>

class DecimationFilter {
public:
    void configure(size_t decimation_factor) {
        m_factor = decimation_factor;
    }

    bool push(float sample, float& output) {
        m_buffer.push_back(sample);

        if (m_buffer.size() >= m_factor) {
            float sum = std::accumulate(m_buffer.begin(), m_buffer.end(), 0.0f);
            output = sum / static_cast<float>(m_factor);
            m_buffer.clear();
            return true;
        }

        return false;
    }

private:
    size_t m_factor = 0;
    std::vector<float> m_buffer;
};
