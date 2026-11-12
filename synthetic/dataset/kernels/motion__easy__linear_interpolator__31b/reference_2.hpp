#pragma once

#include <cstddef>

class LinearInterpolator {
public:
    /**
     * Configures the interpolation parameters. 
     * This is called outside the real-time path.
     */
    void configure(float start_pos, float end_pos, size_t duration_samples) {
        m_start_pos = start_pos;
        m_end_pos = end_pos;
        m_duration = duration_samples;
        m_current_sample = 0;
    }

    /**
     * Advances the interpolation by one sample and returns the position.
     * Real-time safe: no allocation, no locks, no I/O, no exceptions.
     */
    float step() {
        if (m_duration == 0) {
            return m_end_pos;
        }

        if (m_current_sample >= m_duration) {
            return m_end_pos;
        }

        // Use the formula: current_pos = start_pos + (end_pos - start_pos) * (current_sample / duration_samples)
        float progress = static_cast<float>(m_current_sample) / static_cast<float>(m_duration);
        float current_pos = m_start_pos + (m_end_pos - m_start_pos) * progress;

        m_current_sample++;
        return current_pos;
    }

private:
    float m_start_pos = 0.0f;
    float m_end_pos = 0.0f;
    size_t m_duration = 0;
    size_t m_current_sample = 0;
};
