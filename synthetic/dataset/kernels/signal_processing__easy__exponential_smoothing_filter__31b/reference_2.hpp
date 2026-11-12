#pragma once

class ExponentialSmoothingFilter {
public:
    /**
     * @brief Configures the filter with the smoothing factor alpha.
     * @param alpha The smoothing factor (0.0 <= alpha <= 1.0).
     */
    void configure(float alpha) {
        m_alpha = alpha;
        m_prev_output = 0.0f;
    }

    /**
     * @brief Processes a single input sample and returns the filtered result.
     * Real-time safe: No allocations, no exceptions, no locks, no I/O.
     * 
     * @param input The current raw input sample.
     * @return The smoothed output sample.
     */
    float process(float input) noexcept {
        // Formula: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        float current_output = m_alpha * input + (1.0f - m_alpha) * m_prev_output;
        m_prev_output = current_output;
        return current_output;
    }

private:
    float m_alpha{1.0f};
    float m_prev_output{0.0f};
};
