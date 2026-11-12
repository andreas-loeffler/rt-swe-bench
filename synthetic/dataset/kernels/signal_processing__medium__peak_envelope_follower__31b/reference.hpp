#pragma once
#include <cmath>
#include <algorithm>

/**
 * PeakEnvelopeFollower tracks the amplitude envelope of a signal.
 * Domain: signal_processing
 * Difficulty: medium
 */
class PeakEnvelopeFollower {
public:
    void configure(float attack_time_s, float release_time_s, float sample_rate_hz) {
        // coeff = 1 - exp(-1 / (time * fs))
        m_attack_coeff = 1.0f - std::exp(-1.0f / (attack_time_s * sample_rate_hz));
        m_release_coeff = 1.0f - std::exp(-1.0f / (release_time_s * sample_rate_hz));
        m_envelope = 0.0f;
    }

    float process(float input) {
        const float abs_input = std::abs(input);
        
        if (abs_input > m_envelope) {
            m_envelope += m_attack_coeff * (abs_input - m_envelope);
        } else {
            m_envelope += m_release_coeff * (abs_input - m_envelope);
        }
        
        return m_envelope;
    }

    float get_level() const {
        return m_envelope;
    }

    void reset() {
        m_envelope = 0.0f;
    }

private:
    float m_attack_coeff = 0.0f;
    float m_release_coeff = 0.0f;
    float m_envelope = 0.0f;
};
