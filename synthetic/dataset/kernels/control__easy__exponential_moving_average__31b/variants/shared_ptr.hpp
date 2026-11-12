#pragma once
#include <memory>

class ExponentialMovingAverage {
public:
    void configure(double alpha) {
        m_alpha = alpha;
        m_initialized = false;
    }

    double process(double input) noexcept {
        if (!m_initialized) {
            m_state = std::make_unique<double>(input);
            m_initialized = true;
        } else {
            *m_state = (m_alpha * input) + (1.0 - m_alpha) * (*m_state);
        }
        return *m_state;
    }

private:
    double m_alpha = 0.0;
    std::unique_ptr<double> m_state;
    bool m_initialized = false;
};
