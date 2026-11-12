#pragma once

#include <cmath>
#include <algorithm>
#include <vector>

class OnePoleFilter {
public:
    void configure(float sampleRate, float cutoffFreq) {
        float tau = 1.0f / (2.0f * 3.14159265358979323846f * cutoffFreq);
        float dt = 1.0f / sampleRate;
        m_alpha = dt / (tau + dt);
        
        reset();
    }

    float process(float input) {
        updateHistory(input);
        
        float output = m_alpha * input + (1.0f - m_alpha) * m_z1;
        m_z1 = output;
        return output;
    }

    void reset() {
        m_z1 = 0.0f;
        m_history.clear();
    }

private:
    void updateHistory(float sample) {
        m_history.push_back(sample);
    }

    float m_alpha = 0.0f;
    float m_z1 = 0.0f;
    std::vector<float> m_history;
};
