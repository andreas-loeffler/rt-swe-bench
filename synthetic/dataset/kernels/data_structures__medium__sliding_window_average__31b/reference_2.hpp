#pragma once

#include <cstddef>
#include <vector>

class SlidingWindowAverage {
public:
    /**
     * @brief Initializes the internal buffer.
     * This is called outside the real-time thread and can allocate memory.
     */
    void configure(size_t window_size) {
        buffer_.clear();
        buffer_.reserve(window_size);
        buffer_.resize(window_size, 0.0);
        max_size_ = window_size;
        current_sum_ = 0.0;
        count_ = 0;
        head_ = 0;
    }

    /**
     * @brief Adds a new sample to the rolling window in O(1).
     * Real-time safe: No allocation, no locks, no exceptions.
     */
    void push(double sample) noexcept {
        if (max_size_ == 0) return;

        if (count_ < max_size_) {
            // Window is filling up
            buffer_[count_] = sample;
            current_sum_ += sample;
            count_++;
        } else {
            // Window is full, replace oldest sample at current head
            current_sum_ -= buffer_[head_];
            buffer_[head_] = sample;
            current_sum_ += sample;
            head_ = (head_ + 1) % max_size_;
        }
    }

    /**
     * @brief Returns the arithmetic mean of the window in O(1).
     * Real-time safe: No allocation, no locks, no exceptions.
     */
    double get_average() const noexcept {
        if (count_ == 0) {
            return 0.0;
        }
        return current_sum_ / static_cast<double>(count_);
    }

private:
    std::vector<double> buffer_;
    size_t max_size_ = 0;
    size_t count_ = 0;
    size_t head_ = 0;
    double current_sum_ = 0.0;
};
