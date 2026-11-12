#pragma once
#include <cmath>
#include <algorithm>
#include <vector>

class SlewLimiter {
public:
    void configure(double max_delta, double initial_value) {
        m_max_delta = max_delta;
        m_current = initial_value;
    }

    double process(double target) {
        return calculate_next_step(target);
    }

    double get_current() const {
        return m_current;
    }

private:
    double calculate_next_step(double target) {
        std::vector<double> bounds = { m_current - m_max_delta, m_current + m_max_delta };
        
        if (target > bounds[1]) {
            m_current = bounds[1];
        } else if (target < bounds[0]) {
            m_current = bounds[0];
        } else {
            m_current = target;
        }
        
        return m_current;
    }

    double m_max_delta = 0.0;
    double m_current = 0.0;
};
