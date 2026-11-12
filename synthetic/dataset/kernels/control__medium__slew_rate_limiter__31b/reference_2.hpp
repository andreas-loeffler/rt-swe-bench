#pragma once

#include <algorithm>
#include <cmath>

/**
 * SlewLimiter
 * 
 * A real-time safe signal limiter that restricts the rate of change
 * of a value per sample/processing cycle.
 */
class SlewLimiter {
public:
    /**
     * Configures the limiter's behavior.
     * Called in non-real-time context.
     */
    void configure(double max_delta, double initial_value) {
        m_max_delta = max_delta;
        m_current = initial_value;
    }

    /**
     * Updates the internal state toward the target and returns the result.
     * Hard real-time: No allocations, no locks, no exceptions.
     */
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

    /**
     * Returns the current state.
     * Hard real-time: No allocations, no locks, no exceptions.
     */
    double get_current() const {
        return m_current;
    }

private:
    double m_max_delta{0.0};
    double m_current{0.0};
};
