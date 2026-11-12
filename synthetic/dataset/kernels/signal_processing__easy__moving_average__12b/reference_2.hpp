#pragma once
#include <cstddef>
#include <vector>

/**
 * MovingAverage computes the arithmetic mean of a sliding window of samples.
 * 
 * Real-time constraints:
 * - configure(): Non-real-time path. May allocate.
 * - process(): Real-time path. O(1) complexity, no allocation, no locking, no I/O.
 */
class MovingAverage {
public:
    void configure(size_t window_size) {
        // Precondition: window_size >= 1.
        // We allocate the buffer here to avoid allocation in the process() method.
        buffer_.assign(window_size, 0.0);
        max_size_ = window_size;
        
        // Reset state
        sum_ = 0.0;
        index_ = 0;
        count_ = 0;
    }

    double process(double sample) {
        // If the buffer is full, we subtract the oldest sample from the running sum
        if (count_ == max_size_) {
            sum_ -= buffer_[index_];
        } else {
            // Increment count until we reach the window size
            ++count_;
        }

        // Add new sample to the sum and store it in the circular buffer
        buffer_[index_] = sample;
        sum_ += sample;

        // Advance index for the next call (wrap around using modulo)
        index_ = (index_ + 1) % max_size_;

        // Return arithmetic mean: sum divided by number of samples currently in window
        return sum_ / static_cast<double>(count_);
    }

private:
    std::vector<double> buffer_;
    size_t max_size_ = 0;
    size_t index_ = 0;
    size_t count_ = 0;
    double sum_ = 0.0;
};
