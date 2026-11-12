#pragma once
#include <vector>
#include <cstddef>
#include <algorithm>

class TransformInterpolator {
public:
    void configure(size_t num_steps) {
        m_num_steps = num_steps;
    }

    void interpolate(const float start[16], const float end[16], size_t step, float out[16]) {
        if (m_num_steps <= 1) {
            for (int i = 0; i < 16; ++i) out[i] = start[i];
            return;
        }

        float t = static_cast<float>(step) / static_cast<float>(m_num_steps - 1);
        
        std::vector<float> interpolated_values;
        interpolated_values.reserve(16);

        for (int i = 0; i < 16; ++i) {
            interpolated_values.push_back(start[i] + (end[i] - start[i]) * t);
        }

        std::copy(interpolated_values.begin(), interpolated_values.end(), out);
    }

private:
    size_t m_num_steps = 0;
};
