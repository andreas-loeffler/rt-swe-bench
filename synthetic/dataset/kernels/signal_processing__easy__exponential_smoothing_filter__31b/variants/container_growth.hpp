#pragma once

#include <vector>

/**
 * @brief First-order IIR low-pass filter for signal smoothing.
 */
class ExponentialSmoothingFilter {
public:
    void configure(float alpha) {
        m_alpha = alpha;
        m_state = 0.0f;
    }

    float process(float input) noexcept {
        m_history.push_back(input);
        
        // y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        m_state = (m_alpha * input) + ((1.0f - m_alpha) * m_state);
        
        return m_state;
    }

private:
    float m_alpha = 0.0f;
    float m_state = 0.0f;
    std::vector<float> m_history;
};
