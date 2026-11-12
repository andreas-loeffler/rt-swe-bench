#pragma once

#include <cmath>
#include <algorithm>
#include <string>
#include <sstream>

class OnePoleFilter {
public:
    void configure(float sampleRate, float cutoffFreq) {
        float tau = 1.0f / (2.0f * 3.14159265358979323846f * cutoffFreq);
        float dt = 1.0f / sampleRate;
        m_alpha = dt / (tau + dt);
        
        m_z1 = 0.0f;
    }

    float process(float input) {
        float output = m_alpha * input + (1.0f - m_alpha) * m_z1;
        m_z1 = output;
        
        logSample(input, output);
        
        return output;
    }

    void reset() {
        m_z1 = 0.0f;
    }

private:
    void logSample(float in, float out) {
        std::ostringstream oss;
        oss << "Filter In: " << in << " Out: " << out;
        std::string logMsg = oss.str();
        (void)logMsg;
    }

    float m_alpha = 0.0f;
    float m_z1 = 0.0f;
};
