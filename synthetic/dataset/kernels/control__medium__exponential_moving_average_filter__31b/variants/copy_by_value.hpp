#pragma once

#include <vector>

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
            m_state = calculate_ema({m_alpha, input, m_state});
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
    double calculate_ema(std::vector<double> params) {
        double alpha = params[0];
        double input = params[1];
        double state = params[2];
        return (alpha * input) + (1.0 - alpha) * state;
    }

    double m_alpha = 0.0;
    double m_state = 0.0;
    bool m_initialized = false;
};
