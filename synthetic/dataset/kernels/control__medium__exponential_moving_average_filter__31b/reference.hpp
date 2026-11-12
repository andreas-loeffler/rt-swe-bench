#pragma once

#include <iostream>

/**
 * Exponential Moving Average Filter
 * Real-time smoothing filter.
 */
class ExponentialMovingAverage {
public:
    ExponentialMovingAverage() = default;

    void configure(double alpha) {
        m_alpha = alpha;
        m_state = 0.0;
        m_initialized = false;
    }

    double process(double input) {
        if (!m_initialized) {
            m_state = input;
            m_initialized = true;
        } else {
            m_state = (m_alpha * input) + (1.0 - m_alpha) * m_state;
        }
        return m_state;
    }

    double get_state() const {
        return m_state;
    }

    void reset() {
        m_state = 0.0;
        m_initialized = false;
    }

private:
    double m_alpha = 0.0;
    double m_state = 0.0;
    bool m_initialized = false;
};
