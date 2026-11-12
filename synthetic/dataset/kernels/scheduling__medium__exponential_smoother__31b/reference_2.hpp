#pragma once

/**
 * @brief A real-time safe first-order IIR low-pass filter.
 * 
 * The filter follows the difference equation:
 * y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
 * 
 * Real-time constraints:
 * - No heap allocation in push() or get_current().
 * - No exceptions in push() or get_current().
 * - No mutexes/blocking in push() or get_current().
 */
class ExponentialSmoother {
public:
    /**
     * @brief Configures the smoothing factor and initial state.
     * 
     * @param alpha Smoothing factor in the range (0, 1].
     * @param initial_value The starting value of the filter.
     */
    void configure(double alpha, double initial_value) {
        m_alpha = alpha;
        m_current_value = initial_value;
    }

    /**
     * @brief Processes a new input sample.
     * 
     * @param input The raw sensor reading.
     * @return The filtered result.
     */
    double push(double input) {
        // Formula: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        // Optimized: y[n] = y[n-1] + alpha * (x[n] - y[n-1])
        // This version is numerically slightly more stable for small alpha.
        m_current_value += m_alpha * (input - m_current_value);
        return m_current_value;
    }

    /**
     * @brief Retrieves the current smoothed value.
     * 
     * @return The most recent smoothed value.
     */
    double get_current() const {
        return m_current_value;
    }

private:
    double m_alpha = 0.0;
    double m_current_value = 0.0;
};
