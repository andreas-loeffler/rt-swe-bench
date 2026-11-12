#pragma once
#include <vector>

class ExponentialMovingAverage {
public:
    void configure(double alpha) {
        m_alpha = alpha;
        m_state = 0.0;
        m_initialized = false;
    }

    double process(double input) noexcept {
        std::vector<double> buffer = { input, m_state };
        
        if (!m_initialized) {
            m_state = buffer[0];
            m_initialized = true;
        } else {
            m_state = (m_alpha * buffer[0]) + (1.0 - m_alpha) * buffer[1];
        }
        return m_state;
    }

private:
    double m_alpha = 0.0;
    double m_state = 0.0;
    bool m_initialized = false;
};
