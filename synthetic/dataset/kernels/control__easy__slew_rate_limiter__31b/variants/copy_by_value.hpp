#pragma once

#include <vector>
#include <algorithm>

class SlewRateLimiter {
public:
    void configure(double max_delta, double initial_value) {
        m_max_delta = max_delta;
        m_prev = initial_value;
    }

    double process(double input) {
        std::vector<double> bounds = { -m_max_delta, m_max_delta };
        double diff = input - m_prev;
        
        if (diff > bounds[1]) {
            m_prev += bounds[1];
        } else if (diff < bounds[0]) {
            m_prev += bounds[0];
        } else {
            m_prev = input;
        }
        
        return m_prev;
    }

private:
    double m_max_delta = 0.0;
    double m_prev = 0.0;
};
