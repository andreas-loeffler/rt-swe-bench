#pragma once

/**
 * RateDivider
 * 
 * Transforms a high-frequency master clock trigger into a lower-frequency trigger.
 * Uses an accumulator to handle non-integer ratios precisely by preserving the
 * remainder across cycles.
 */
class RateDivider {
public:
    /**
     * Sets up the divider.
     * 
     * @param master_rate Frequency of the calling process.
     * @param target_rate Desired output frequency.
     */
    void configure(double master_rate, double target_rate) {
        // Preconditions are assumed to be met as per requirements.
        // Step size represents the fraction of the period consumed per master tick.
        step_size_ = target_rate / master_rate;
        accumulator_ = 0.0;
    }

    /**
     * Called at every cycle of the master clock.
     * 
     * @return true if the target rate interval has elapsed.
     */
    bool process() {
        accumulator_ += step_size_;
        
        // Handle wrap-around. While a while loop is safer for massive 
        // ratios, the precondition target_rate <= master_rate ensures 
        // a simple if or a single subtraction is sufficient.
        if (accumulator_ >= 1.0) {
            accumulator_ -= 1.0;
            return true;
        }
        
        return false;
    }

    /**
     * Returns the current normalized phase of the divider.
     * 
     * @return Value in the range [0.0, 1.0).
     */
    double get_phase() const {
        return accumulator_;
    }

private:
    double step_size_ = 0.0;
    double accumulator_ = 0.0;
};
