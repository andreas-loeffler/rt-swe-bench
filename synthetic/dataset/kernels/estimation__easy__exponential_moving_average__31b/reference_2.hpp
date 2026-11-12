#pragma once

#include <stdexcept>

class ExponentialMovingAverage {
public:
    /**
     * @brief Prepares the filter with a smoothing factor.
     * @param alpha Smoothing factor in range [0.0, 1.0].
     */
    void configure(float alpha) {
        if (alpha < 0.0f || alpha > 1.0f) {
            throw std::invalid_argument("Alpha must be in range [0.0, 1.0]");
        }
        m_alpha = alpha;
        m_initialized = false;
    }

    /**
     * @brief Processes a single sample through the EMA filter.
     * Real-time safe: no allocation, no locking, no exceptions, no I/O.
     * 
     * @param input The current input sample.
     * @return The smoothed output sample.
     */
    float process(float input) {
        if (!m_initialized) {
            m_current_estimate = input;
            m_initialized = true;
            return m_current_estimate;
        }

        // y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        m_current_estimate = (m_alpha * input) + (1.0f - m_alpha) * m_current_estimate;
        return m_current_estimate;
    }

private:
    float m_alpha = 1.0f;
    float m_current_estimate = 0.0f;
    bool m_initialized = false;
};
