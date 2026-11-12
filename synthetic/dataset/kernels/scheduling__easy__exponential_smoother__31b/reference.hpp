#pragma once

#include <iostream>

class ExponentialSmoother {
public:
    void configure(double alpha) {
        m_alpha = alpha;
        m_initialized = false;
        m_state = 0.0;
    }

    double process(double input) noexcept {
        if (!m_initialized) {
            m_state = input;
            m_initialized = true;
            return m_state;
        }
        
        m_state = m_alpha * input + (1.0 - m_alpha) * m_state;
        return m_state;
    }

private:
    double m_alpha = 1.0;
    double m_state = 0.0;
    bool m_initialized = false;
};
