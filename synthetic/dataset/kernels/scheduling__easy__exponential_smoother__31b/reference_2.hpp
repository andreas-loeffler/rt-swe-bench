#pragma once

class ExponentialSmoother {
public:
    /**
     * @brief Configures the smoothing factor and resets state.
     * @param alpha Smoothing factor in the range [0.0, 1.0].
     */
    void configure(double alpha) {
        m_alpha = alpha;
        m_initialized = false;
        m_prev_output = 0.0;
    }

    /**
     * @brief Processes a sample through the IIR filter.
     * @param input The raw input sample.
     * @return The smoothed output value.
     * 
     * Real-time Constraint: No heap allocation, no exceptions, 
     * no mutex locks, no I/O.
     */
    double process(double input) noexcept {
        if (!m_initialized) {
            m_prev_output = input;
            m_initialized = true;
            return m_prev_output;
        }

        // y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        double output = m_alpha * input + (1.0 - m_alpha) * m_prev_output;
        m_prev_output = output;
        return output;
    }

private:
    double m_alpha{1.0};
    double m_prev_output{0.0};
    bool m_initialized{false};
};
