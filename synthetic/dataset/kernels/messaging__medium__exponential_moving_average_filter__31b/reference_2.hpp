#pragma once

class ExponentialMovingAverageFilter {
public:
    /**
     * @brief Configures the filter with a smoothing factor.
     * @param alpha The smoothing factor (0.0 < alpha <= 1.0).
     * Precondition: alpha must be in the range (0, 1].
     */
    void configure(double alpha) {
        m_alpha = alpha;
        reset();
    }

    /**
     * @brief Integrates a new sample into the filter.
     * Updates the internal state. 
     * Real-time safe: no allocation, no locking, no I/O.
     */
    void push(double value) {
        if (!m_initialized) {
            m_current_value = value;
            m_initialized = true;
        } else {
            m_current_value = m_alpha * value + (1.0 - m_alpha) * m_current_value;
        }
    }

    /**
     * @brief Returns the current filtered value.
     * If no samples have been pushed, returns 0.0.
     * Real-time safe: no allocation, no locking, no I/O.
     */
    double get_value() const {
        return m_initialized ? m_current_value : 0.0;
    }

    /**
     * @brief Resets the internal state.
     * Real-time safe: no allocation, no locking, no I/O.
     */
    void reset() {
        m_current_value = 0.0;
        m_initialized = false;
    }

private:
    double m_alpha{0.5};
    double m_current_value{0.0};
    bool m_initialized{false};
};
