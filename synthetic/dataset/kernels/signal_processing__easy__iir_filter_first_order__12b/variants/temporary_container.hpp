#pragma once

#include <vector>

class IirFilter {
public:
    void configure(float alpha) {
        m_alpha = alpha;
        m_previous_output = 0.0f;
    }

    float process(float input) {
        std::vector<float> buffer;
        buffer.reserve(2);
        buffer.push_back(m_alpha * input);
        buffer.push_back((1.0f - m_alpha) * m_previous_output);
        
        float output = buffer[0] + buffer[1];
        m_previous_output = output;
        return output;
    }

private:
    float m_alpha = 0.0f;
    float m_previous_output = 0.0f;
};
