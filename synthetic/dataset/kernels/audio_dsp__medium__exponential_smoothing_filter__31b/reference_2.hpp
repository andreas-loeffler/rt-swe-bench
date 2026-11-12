#pragma once

#include <cmath>

class OnePoleFilter {
public:
    /**
     * Configures the filter coefficients.
     * The formula for a one-pole low-pass filter is:
     * y[n] = (1 - alpha) * y[n-1] + alpha * x[n]
     * where alpha = dt / (tau + dt)
     * and tau = 1 / (2 * pi * cutoffFreq)
     * dt = 1 / sampleRate
     */
    void configure(float sampleRate, float cutoffFreq) {
        // Preconditions are handled by the caller according to spec,
        // but we ensure valid math.
        float dt = 1.0f / sampleRate;
        float tau = 1.0f / (2.0f * M_PI * cutoffFreq);
        alpha_ = dt / (tau + dt);

        // Clamp alpha to [0, 1] for stability
        if (alpha_ > 1.0f) alpha_ = 1.0f;
        if (alpha_ < 0.0f) alpha_ = 0.0f;
    }

    /**
     * Processes a single sample.
     * This method is real-time safe: no allocation, no locks, no exceptions.
     */
    float process(float input) {
        // y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        float output = alpha_ * input + (1.0f - alpha_) * last_output_;
        last_output_ = output;
        return output;
    }

    /**
     * Resets the internal state to zero.
     * This method is real-time safe.
     */
    void reset() {
        last_output_ = 0.0f;
    }

private:
    float alpha_ = 0.0f;
    float last_output_ = 0.0f;
};
