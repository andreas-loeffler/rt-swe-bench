#pragma once

#include <cmath>
#include <algorithm>

class OnePoleFilter {
public:
    void configure(float sampleRate, float cutoffFreq) {
        // Standard calculation for one-pole low-pass coefficient
        // omega = 2 * pi * fc / fs
        // alpha = omega / (omega + tan(omega/2)) 
        // Or simpler approximation for DSP:
        float omega = 2.0f * 3.14159265358979323846f * cutoffFreq / sampleRate;
        m_alpha = omega / (omega + 1.0f); // Simplified one-pole mapping
        // For a more precise RC filter:
        // m_alpha = 1.0f - std::exp(-2.0f * 3.14159265358979323846f * cutoffFreq / sampleRate);
        
        // Re-implementing as standard alpha = dt / (tau + dt)
        float tau = 1.0f / (2.0f * 3.14159265358979323846f * cutoffFreq);
        float dt = 1.0f / sampleRate;
        m_alpha = dt / (tau + dt);
        
        m_z1 = 0.0f;
    }

    float process(float input) {
        // Difference equation: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        float output = m_alpha * input + (1.0f - m_alpha) * m_z1;
        m_z1 = output;
        return output;
    }

    void reset() {
        m_z1 = 0.0f;
    }

private:
    float m_alpha = 0.0f;
    float m_z1 = 0.0f;
};
