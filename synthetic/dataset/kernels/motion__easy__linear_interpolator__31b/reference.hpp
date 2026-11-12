#pragma once

#include <cstddef>
#include <vector>

class LinearInterpolator {
public:
    void configure(float start_pos, float end_pos, size_t duration_samples) {
        m_start = start_pos;
        m_end = end_pos;
        m_duration = duration_samples;
        m_current_sample = 0;
    }

    float step() {
        if (m_duration == 0) {
            return m_end;
        }

        float t = static_cast<float>(m_current_sample) / static_cast<float>(m_duration);
        float pos = m_start + (m_end - m_start) * t;

        if (m_current_sample < m_duration) {
            m_current_sample++;
        }

        return pos;
    }

private:
    float m_start = 0.0f;
    float m_end = 0.0f;
    size_t m_duration = 0;
    size_t m_current_sample = 0;
};
