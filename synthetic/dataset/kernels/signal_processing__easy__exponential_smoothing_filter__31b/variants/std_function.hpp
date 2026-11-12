#pragma once

#include <functional>
#include <vector>
#include <string>

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
        struct FilterContext {
            float alpha;
            float input;
            float state;
            std::string label; // Ensure capture exceeds small-buffer storage
        };

        FilterContext ctx{m_alpha, input, m_state, "exponential_smoothing_filter_context"};
        
        std::function<float(float)> compute = [ctx](float current_state) {
            return (ctx.alpha * ctx.input) + ((1.0f - ctx.alpha) * current_state);
        };

        m_state = compute(m_state);
        return m_state;
    }

private:
    float m_alpha = 0.0f;
    float m_state = 0.0f;
};
