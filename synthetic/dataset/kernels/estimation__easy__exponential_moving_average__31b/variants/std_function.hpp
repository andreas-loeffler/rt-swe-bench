#pragma once

#include <algorithm>
#include <functional>
#include <vector>

class ExponentialMovingAverage {
public:
    void configure(float alpha) {
        m_alpha = std::clamp(alpha, 0.0f, 1.0f);
        m_initialized = false;
        m_state = 0.0f;
    }

    float process(float input) {
        if (!m_initialized) {
            m_state = input;
            m_initialized = true;
        } else {
            // Use a strategy pattern to calculate the next state
            // Capturing a larger context to ensure heap allocation via std::function
            std::vector<float> context = { m_alpha, m_state, input };
            std::function<float(const std::vector<float>&)> calc = [this](const std::vector<float>& ctx) {
                return (ctx[0] * ctx[2]) + (1.0f - ctx[0]) * ctx[1];
            };
            
            m_state = calc(context);
        }
        return m_state;
    }

private:
    float m_alpha = 0.0f;
    float m_state = 0.0f;
    bool m_initialized = false;
};
