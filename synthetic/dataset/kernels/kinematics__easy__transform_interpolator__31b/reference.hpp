#pragma once
#include <vector>
#include <cstddef>

class TransformInterpolator {
public:
    void configure(size_t num_steps) {
        m_num_steps = num_steps;
        // Buffer not strictly required by logic but reserved as per "one internal buffer"
        m_buffer.resize(num_steps); 
    }

    void interpolate(const float start[16], const float end[16], size_t step, float out[16]) {
        if (m_num_steps <= 1) {
            for (int i = 0; i < 16; ++i) out[i] = start[i];
            return;
        }

        float t = static_cast<float>(step) / static_cast<float>(m_num_steps - 1);
        for (int i = 0; i < 16; ++i) {
            out[i] = start[i] + (end[i] - start[i]) * t;
        }
    }

private:
    size_t m_num_steps = 0;
    std::vector<float> m_buffer;
};
