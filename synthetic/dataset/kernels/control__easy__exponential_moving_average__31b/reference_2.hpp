#pragma once

class ExponentialMovingAverage {
public:
    /**
     * @brief Configures the smoothing factor alpha.
     * @param alpha The smoothing factor, typically in the range (0.0, 1.0].
     */
    void configure(double alpha) {
        m_alpha = alpha;
        m_initialized = false;
    }

    /**
     * @brief Updates the internal state with a new input and returns the filtered value.
     * 
     * This method is hard real-time: no heap allocation, no locks, and no exceptions.
     * 
     * @param input The current scalar input.
     * @return The filtered output value.
     */
    double process(double input) noexcept {
        // If this is the first call, we initialize the previous_output 
        // to the current input to avoid starting from zero.
        if (!m_initialized) {
            m_previous_output = input;
            m_initialized = true;
            return m_previous_output;
        }

        // Formula: output = (alpha * input) + ((1.0 - alpha) * previous_output)
        double output = (m_alpha * input) + ((1.0 - m_alpha) * m_previous_output);
        m_previous_output = output;
        return output;
    }

private:
    double m_alpha = 1.0;
    double m_previous_output = 0.0;
    bool m_initialized = false;
};
