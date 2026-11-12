#pragma once

#include <iostream>

class ExponentialSmoother {
public:
    void configure(double alpha, double initial_value) {
        m_alpha = alpha;
        m_state = initial_value;
        m_initialized = true;
    }

    double push(double input) {
        // y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        m_state = (m_alpha * input) + (1.0 - m_alpha) * m_state;
        return m_state;
    }

    double get_current() const {
        return m_state;
    }

private:
    double m_alpha = 0.0;
    double m_state = 0.0;
    bool m_initialized = false;
};
