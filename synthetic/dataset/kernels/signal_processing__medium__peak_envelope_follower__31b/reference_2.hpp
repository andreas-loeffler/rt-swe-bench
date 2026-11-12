#pragma once

#include <cmath>
#include <algorithm>

class PeakEnvelopeFollower {
public:
    /**
     * Configures the coefficients for the envelope follower.
     * Calculations are performed here as this is not a real-time path.
     */
    void configure(float attack_time_s, float release_time_s, float sample_rate_hz) {
        // Coefficient formula: coeff = 1.0 - exp(-1.0 / (time * sample_rate))
        // We use std::exp for high precision during configuration.
        
        float attack_denom = attack_time_s * sample_rate_hz;
        float release_denom = release_time_s * sample_rate_hz;

        // Prevent division by zero if time or sample_rate is 0
        if (attack_denom <= 0.0f) {
            attack_coeff_ = 1.0f;
        } else {
            attack_coeff_ = 1.0f - std::exp(-1.0f / attack_denom);
        }

        if (release_denom <= 0.0f) {
            release_coeff_ = 1.0f;
        } else {
            release_coeff_ = 1.0f - std::exp(-1.0f / release_denom);
        }
    }

    /**
     * Processes a single sample and returns the updated envelope.
     * This method is hard real-time: no allocations, no locks, no exceptions.
     */
    float process(float input) {
        float x = std::abs(input);
        
        if (x > envelope_) {
            envelope_ += attack_coeff_ * (x - envelope_);
        } else {
            envelope_ += release_coeff_ * (x - envelope_);
        }

        return envelope_;
    }

    /**
     * Returns the current envelope level without updating state.
     */
    float get_level() const {
        return envelope_;
    }

    /**
     * Resets the internal envelope state to zero.
     */
    void reset() {
        envelope_ = 0.0f;
    }

private:
    float envelope_ = 0.0f;
    float attack_coeff_ = 0.0f;
    float release_coeff_ = 0.0f;
};
