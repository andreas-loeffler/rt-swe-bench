#pragma once

#include <cstddef>
#include <vector>

class JointVelocityFilter {
public:
    /**
     * @brief Initializes the filter with a specific window size.
     * Memory allocation is allowed here as it is called during the 
     * non-real-time initialization phase.
     */
    void configure(size_t window_size) {
        max_window_size_ = window_size;
        buffer_.clear();
        // Use resize or reserve and ensure we don't exceed capacity.
        // To satisfy the no-allocation constraint, we pre-allocate.
        buffer_.resize(max_window_size_);
        current_sum_ = 0.0;
        head_ = 0;
        count_ = 0;
    }

    /**
     * @brief Processes a new sample in real-time.
     * Constraint: No heap allocation, no mutexes, no exceptions.
     * 
     * @param sample The new joint encoder reading.
     * @return The arithmetic mean of the current buffer.
     */
    double process(double sample) {
        if (max_window_size_ == 0) {
            return 0.0;
        }

        if (count_ < max_window_size_) {
            buffer_[head_] = sample;
            current_sum_ += sample;
            count_++;
            head_ = (head_ + 1) % max_window_size_;
        } else {
            // Buffer is full, subtract the oldest (at head_) and replace
            current_sum_ -= buffer_[head_];
            buffer_[head_] = sample;
            current_sum_ += sample;
            head_ = (head_ + 1) % max_window_size_;
        }

        return current_sum_ / static_cast<double>(count_);
    }

private:
    std::vector<double> buffer_;
    size_t max_window_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    double current_sum_ = 0.0;
};
