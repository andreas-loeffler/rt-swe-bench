#pragma once

#include <algorithm>

class ExponentialMovingAverage {
public:
    void configure(float alpha) {
        m_alpha = std::clamp(alpha, 0.0f, 1.0f);
        m_initialized = false;
        m_state = 0.0f;
    }

    float process(float input) {
        if (!m_initialized) {
            m_state = input;
            m_initialized = true;
        } else {
            m_state = (m_alpha * input) + (1.0f - m_alpha) * m_state;
        }
        return m_state;
    }

private:
    float m_alpha = 0.0f;
    float m_state = 0.0f;
    bool m_initialized = false;
};
