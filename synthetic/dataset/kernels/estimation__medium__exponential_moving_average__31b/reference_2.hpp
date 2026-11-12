#pragma once

class ExponentialMovingAverage {
public:
    /**
     * Configures the filter smoothing factor.
     * @param alpha The smoothing factor \alpha \in (0, 1].
     */
    void configure(double alpha) {
        m_alpha = alpha;
        m_current_value = 0.0;
        m_initialized = false;
    }

    /**
     * Feeds a new sample into the filter to update the internal state.
     * Formula: S_t = \alpha \cdot x_t + (1 - \alpha) \cdot S_{t-1}
     * If first sample, S_0 = x_0.
     */
    void push(double sample) {
        // Real-time safe: No allocation, no locks, no exceptions.
        if (!m_initialized) {
            m_current_value = sample;
            m_initialized = true;
        } else {
            m_current_value = m_alpha * sample + (1.0 - m_alpha) * m_current_value;
        }
    }

    /**
     * Returns the current smoothed value of the filter.
     * Returns 0.0 if pop is called before any push.
     */
    double pop() const {
        // Real-time safe: No allocation, no locks, no exceptions.
        return m_current_value;
    }

private:
    double m_alpha = 0.0;
    double m_current_value = 0.0;
    bool m_initialized = false;
};
