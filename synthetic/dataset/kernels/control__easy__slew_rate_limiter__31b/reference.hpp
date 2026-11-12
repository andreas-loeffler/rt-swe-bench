#pragma once

class SlewRateLimiter {
public:
    void configure(double max_delta, double initial_value) {
        m_max_delta = max_delta;
        m_prev = initial_value;
    }

    double process(double input) {
        double diff = input - m_prev;
        if (diff > m_max_delta) {
            m_prev += m_max_delta;
        } else if (diff < -m_max_delta) {
            m_prev -= m_max_delta;
        } else {
            m_prev = input;
        }
        return m_prev;
    }

private:
    double m_max_delta = 0.0;
    double m_prev = 0.0;
};
