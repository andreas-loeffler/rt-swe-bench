#pragma once

/**
 * @brief First-Order Infinite Impulse Response (IIR) Filter.
 * 
 * Implements a basic exponential moving average / low-pass filter:
 * y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
 */
class IirFilter {
public:
    /**
     * Configures the filter coefficients.
     * @param alpha The smoothing factor (typically 0 < alpha < 1).
     */
    void configure(float alpha) {
        m_alpha = alpha;
        m_state = 0.0f;
        m_initialized = true;
    }

    /**
     * Processes a single sample through the filter.
     * This method is real-time safe: no allocation, locking, or I/O.
     * @param input The raw input sample.
     * @return The filtered output sample.
     */
    float process(float input) {
        // Note: The precondition states configure must be called first.
        // We maintain a state variable for the previous output.
        if (!m_initialized) {
            return input;
        }

        // Difference equation: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        m_state = m_alpha * input + (1.0f - m_alpha) * m_state;
        return m_state;
    }

private:
    float m_alpha = 0.0f;
    float m_state = 0.0f;
    bool m_initialized = false;
};
