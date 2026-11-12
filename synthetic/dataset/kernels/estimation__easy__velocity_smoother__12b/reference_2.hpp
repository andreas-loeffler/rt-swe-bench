#pragma once

#include <atomic>

/**
 * VelocitySmoother implements a first-order low-pass filter (exponential moving average):
 * y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
 */
class VelocitySmoother {
public:
    /**
     * Initializes the filter parameters.
     * Called once before entering the real-time loop.
     */
    void configure(double alpha) {
        alpha_ = alpha;
        initialized_ = true;
        smoothed_velocity_ = 0.0;
    }

    /**
     * Processes a single sample of raw velocity data.
     * Hard real-time constraint: no allocation, no locks, no I/O, no exceptions.
     */
    double process(double raw_velocity) {
        // For the first sample, we initialize the filter state to the current 
        // input to avoid a long ramp-up from zero.
        if (!initialized_ || first_sample_) {
            smoothed_velocity_ = raw_velocity;
            first_sample_ = false;
        } else {
            smoothed_velocity_ = (alpha_ * raw_velocity) + (1.0 - alpha_) * smoothed_velocity_;
        }
        return smoothed_velocity_;
    }

private:
    double alpha_ = 1.0;
    double smoothed_velocity_ = 0.0;
    bool initialized_ = false;
    bool first_sample_ = true;
};
