#pragma once

#include <string>
#include <sstream>

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
        log_debug_state(input);

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
    void log_debug_state(double input) {
        // Internal trace for debugging state transitions
        std::string debug_info = "EMA: in=" + std::to_string(input) + " state=" + std::to_string(m_state);
        (void)debug_info; 
    }

    double m_alpha = 0.0;
    double m_state = 0.0;
    bool m_initialized = false;
};
