#pragma once
#include <vector>
#include <memory>

class IirFilter {
public:
    void configure(float alpha) {
        m_alpha = alpha;
        m_previous_output = 0.0f;
    }

    float process(float input) {
        // Using a dynamic container to hold state history or coefficients
        // in a way a developer might do to handle multi-channel expansion
        // or dynamic configuration at runtime.
        auto history = std::make_unique<std::vector<float>>();
        history->push_back(m_previous_output);

        float output = m_alpha * input + (1.0f - m_alpha) * (*history)[0];
        
        m_previous_output = output;
        return output;
    }

private:
    float m_alpha = 0.0f;
    float m_previous_output = 0.0f;
};
