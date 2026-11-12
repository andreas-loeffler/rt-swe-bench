#pragma once

#include <cmath>
#include <numbers>

/**
 * SineWaveOscillator
 * 
 * Implements a basic phase-accumulator based sine wave generator.
 * The phase is kept in the range [0, 2*PI).
 */
class SineWaveOscillator {
public:
    /**
     * Configures the oscillator parameters.
     * This method is called on the non-real-time path.
     */
    void configure(double sampleRate, double frequency) {
        // Preconditions: sampleRate > 0, frequency >= 0
        // Phase increment per sample: (2 * PI * frequency) / sampleRate
        mPhaseIncrement = (2.0 * std::numbers::pi * frequency) / sampleRate;
        mPhase = 0.0;
    }

    /**
     * Generates the next sample.
     * This method is called on the hard real-time path.
     */
    double process() {
        // Calculate current amplitude
        double sample = std::sin(mPhase);

        // Advance phase
        mPhase += mPhaseIncrement;

        // Wrap phase to keep it within [0, 2*PI) to maintain precision over long durations
        if (mPhase >= 2.0 * std::numbers::pi) {
            mPhase -= 2.0 * std::numbers::pi;
        }

        return sample;
    }

private:
    double mPhase = 0.0;
    double mPhaseIncrement = 0.0;
};
