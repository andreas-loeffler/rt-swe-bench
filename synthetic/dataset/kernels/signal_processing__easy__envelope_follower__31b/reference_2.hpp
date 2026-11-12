#pragma once

#include <cmath>
#include <algorithm>

class EnvelopeFollower {
public:
    void configure(float sampleRate, float attackTimeSec, float releaseTimeSec) {
        m_sampleRate = sampleRate;
        
        // Calculate coefficients: alpha = 1.0 - exp(-1 / (tau * fs))
        // We use 1.0f - exp(...) for the smooth transition towards the target.
        m_alphaAttack = 1.0f - std::exp(-1.0f / (attackTimeSec * m_sampleRate));
        m_alphaRelease = 1.0f - std::exp(-1.0f / (releaseTimeSec * m_sampleRate));
        
        // Ensure coefficients are clamped to a valid range [0, 1]
        m_alphaAttack = std::max(0.0f, std::min(1.0f, m_alphaAttack));
        m_alphaRelease = std::max(0.0f, std::min(1.0f, m_alphaRelease));
        
        m_currentEnvelope = 0.0f;
    }

    float step(float input) noexcept {
        // Rectify the input
        float rectified = std::abs(input);
        
        // Determine which coefficient to use based on the trend
        // If current envelope is lower than rectified, we are "attacking"
        // If current envelope is higher than rectified, we are "releasing"
        float alpha = (rectified > m_currentEnvelope) ? m_alphaAttack : m_alphaRelease;
        
        // Linear interpolation: y = y_prev + alpha * (target - y_prev)
        m_currentEnvelope += alpha * (rectified - m_currentEnvelope);
        
        return m_currentEnvelope;
    }

private:
    float m_sampleRate = 44100.0f;
    float m_alphaAttack = 0.0f;
    float m_alphaRelease = 0.0f;
    float m_currentEnvelope = 0.0f;
};
