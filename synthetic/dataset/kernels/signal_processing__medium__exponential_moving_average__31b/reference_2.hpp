#pragma once

#include <cmath>
#include <algorithm>

class ExponentialMovingAverage {
public:
    /**
     * Prepares the filter for operation.
     * Calculates alpha based on the formula: alpha = 1 - exp(-1 / (sample_rate * tau))
     */
    void configure(double sample_rate, double time_constant) {
        m_alpha = 1.0 - std::exp(-1.0 / (sample_rate * time_constant));
        reset();
    }

    /**
     * Processes a single input sample and returns the updated filtered value.
     * Real-time safe: No heap allocation, no locks, no exceptions.
     */
    double step(double input) {
        m_current_value = (m_alpha * input) + (1.0 - m_alpha) * m_current_value;
        return m_current_value;
    }

    /**
     * Returns the most recently computed filtered value.
     * Real-time safe: No heap allocation, no locks, no exceptions.
     */
    double get_current_value() const {
        return m_current_value;
    }

    /**
     * Resets the internal state to zero.
     * Real-time safe: No heap allocation, no locks, no exceptions.
     */
    void reset() {
        m_current_value = 0.0;
    }

private:
    double m_alpha{0.0};
    double m_current_value{0.0};
};
