#pragma once
#include <cstddef>

class PulseCounter {
public:
    /**
     * Initializes the kernel.
     * @param max_threshold The maximum value at which the internal counter will trigger a "high" state.
     */
    void configure(size_t max_threshold) {
        max_threshold_ = max_threshold;
        count_ = 0;
    }

    /**
     * Processes a new sample of the signal.
     * 
     * Constraints: Real-time method. No allocation, no exceptions, no locks, no I/O.
     * 
     * @param current_state The current digital value of the signal.
     * @param previous_state The value of the signal in the previous call.
     * @return true if a rising edge was detected AND the counter is <= max_threshold.
     */
    bool process(bool current_state, bool previous_state) {
        // Detect rising edge: transition from false (0) to true (1)
        if (!previous_state && current_state) {
            count_++;
            
            // Return true if the count is within the threshold inclusive
            return count_ <= max_threshold_;
        }
        
        return false;
    }

private:
    size_t max_threshold_ = 0;
    size_t count_ = 0;
};
