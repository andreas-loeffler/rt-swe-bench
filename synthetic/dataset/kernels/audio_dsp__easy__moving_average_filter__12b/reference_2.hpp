#pragma once
#include <cstddef>
#include <vector>
#include <numeric>

class MovingAverageFilter {
public:
    /**
     * Initializes the internal buffer.
     * Allocation is permitted here as this is called outside the real-time loop.
     */
    void configure(size_t window_size) {
        // Precondition: window_size > 0 is assumed per requirements
        buffer_.assign(window_size, 0.0f);
        max_size_ = window_size;
        current_size_ = 0;
        write_index_ = 0;
        sum_ = 0.0f;
    }

    /**
     * Processes a single input sample and returns the current average.
     * Real-time constraints: No allocation, no locking, no I/O.
     * Complexity: O(1)
     */
    float process(float sample) {
        if (max_size_ == 0) return 0.0f;

        if (current_size_ < max_size_) {
            // Buffer is filling up: just add the sample to the sum
            sum_ += sample;
            buffer_[write_index_] = sample;
            current_size_++;
        } else {
            // Buffer is full: subtract the oldest sample and add the newest
            sum_ -= buffer_[write_index_];
            sum_ += sample;
            buffer_[write_index_] = sample;
        }

        // Move index forward in a circular fashion
        write_index_ = (write_index_ + 1) % max_size_;

        return sum_ / static_cast<float>(current_size_);
    }

private:
    std::vector<float> buffer_;
    size_t max_size_ = 0;
    size_t current_size_ = 0;
    size_t write_index_ = 0;
    double sum_ = 0.0; // Use double for the accumulator to mitigate precision drift
};
