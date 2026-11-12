#pragma once

#include <cstddef>
#include <vector>

class MovingAverageFilter {
public:
    /**
     * @brief Initializes the filter with a window size of N.
     * May allocate memory as this is called outside the real-time path.
     */
    void configure(size_t window_size) {
        buffer_.clear();
        buffer_.reserve(window_size);
        max_size_ = window_size;
        current_sum_ = 0.0;
    }

    /**
     * @brief Adds a new sample and returns the average of the current window.
     * Hard real-time: no allocations, no locks, no exceptions.
     */
    double process(double sample) {
        if (max_size_ == 0) {
            return 0.0;
        }

        // Handle initial phase (fewer than max_size samples)
        if (buffer_.size() < max_size_) {
            buffer_.push_back(sample);
            current_sum_ += sample;
        } else {
            // Sliding window: subtract oldest, add newest
            current_sum_ -= buffer_[head_];
            buffer_[head_] = sample;
            current_sum_ += sample;
            
            // Increment head and wrap around (circular buffer)
            head_ = (head_ + 1) % max_size_;
        }

        return current_sum_ / static_cast<double>(buffer_.size());
    }

private:
    std::vector<double> buffer_;
    size_t max_size_ = 0;
    size_t head_ = 0;
    double current_sum_ = 0.0;
};
