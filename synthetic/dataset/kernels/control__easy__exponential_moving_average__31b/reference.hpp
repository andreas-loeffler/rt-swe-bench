#pragma once
#include <cmath>

class ExponentialMovingAverage {
public:
    void configure(double alpha) {
        m_alpha = alpha;
        m_state = 0.0;
        m_initialized = false;
    }

    double process(double input) noexcept {
        if (!m_initialized) {
            m_state = input;
            m_initialized = true;
        } else {
            m_state = (m_alpha * input) + (1.0 - m_alpha) * m_state;
        }
        return m_state;
    }

private:
    double m_alpha = 0.0;
    double m_state = 0.0;
    bool m_initialized = false;
};
