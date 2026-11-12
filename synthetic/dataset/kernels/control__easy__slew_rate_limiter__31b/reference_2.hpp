#pragma once

class SlewRateLimiter {
public:
    /**
     * @brief Configures the limiter parameters.
     * @param max_delta The maximum allowed change per sample.
     * @param initial_value The starting value of the output.
     */
    void configure(double max_delta, double initial_value) {
        m_max_delta = max_delta;
        m_prev_output = initial_value;
    }

    /**
     * @brief Processes a single sample with slew rate limiting.
     * @param input The target input value.
     * @return The slewed output value.
     * 
     * This method is hard real-time: no allocations, no locks, no exceptions.
     */
    double process(double input) {
        double diff = input - m_prev_output;

        if (diff > m_max_delta) {
            m_prev_output += m_max_delta;
        } else if (diff < -m_max_delta) {
            m_prev_output -= m_max_delta;
        } else {
            m_prev_output = input;
        }

        return m_prev_output;
    }

private:
    double m_max_delta{0.0};
    double m_prev_output{0.0};
};
