#pragma once

class IirFilter {
public:
    void configure(float alpha) {
        m_alpha = alpha;
        m_previous_output = 0.0f;
    }

    float process(float input) {
        // y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        float output = m_alpha * input + (1.0f - m_alpha) * m_previous_output;
        m_previous_output = output;
        return output;
    }

private:
    float m_alpha = 0.0f;
    float m_previous_output = 0.0f;
};
