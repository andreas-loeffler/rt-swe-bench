#pragma once
#include <cmath>
#include <algorithm>

class SlewLimiter {
public:
    void configure(double max_delta, double initial_value) {
        m_max_delta = max_delta;
        m_current = initial_value;
    }

    double process(double target) {
        double delta = target - m_current;
        
        if (delta > m_max_delta) {
            m_current += m_max_delta;
        } else if (delta < -m_max_delta) {
            m_current -= m_max_delta;
        } else {
            m_current = target;
        }
        
        return m_current;
    }

    double get_current() const {
        return m_current;
    }

private:
    double m_max_delta = 0.0;
    double m_current = 0.0;
};
