#pragma once

class ExponentialMovingAverage {
public:
    /**
     * @brief Initializes the filter with the smoothing factor alpha.
     * @param alpha The weight for the current input (0.0 <= alpha <= 1.0).
     */
    void configure(double alpha) {
        m_alpha = alpha;
        reset();
    }

    /**
     * @brief Updates the internal state with a new sample and returns the filtered value.
     * @param input The current raw sample.
     * @return The filtered value y[n].
     */
    double process(double input) {
        // Handle first sample initialization
        if (!m_initialized) {
            m_state = input;
            m_initialized = true;
            return m_state;
        }

        // Formula: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        // Optimized as: y[n] = y[n-1] + alpha * (x[n] - y[n-1])
        // This reduces the number of multiplications.
        m_state = m_state + m_alpha * (input - m_state);
        return m_state;
    }

    /**
     * @brief Returns the current internal state without updating it.
     */
    double get_state() const {
        return m_state;
    }

    /**
     * @brief Resets the internal state to 0.0 and clears initialization flag.
     */
    void reset() {
        m_state = 0.0;
        m_initialized = false;
    }

private:
    double m_alpha = 0.0;
    double m_state = 0.0;
    bool m_initialized = false;
};
