#pragma once

#include <vector>
#include <cstddef>
#include <functional>
#include <string>

class DecimationFilter {
public:
    void configure(size_t decimation_factor) {
        m_factor = decimation_factor;
        m_sum = 0.0f;
        m_count = 0;
    }

    bool push(float sample, float& output) {
        m_sum += sample;
        m_count++;

        if (m_count >= m_factor) {
            std::string metadata = "DecimationFactor:" + std::to_string(m_factor);
            float current_sum = m_sum;
            size_t current_factor = m_factor;
            
            std::function<void(float&)> calculate_average = [metadata, current_sum, current_factor](float& out) {
                out = current_sum / static_cast<float>(current_factor);
            };

            calculate_average(output);
            
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
