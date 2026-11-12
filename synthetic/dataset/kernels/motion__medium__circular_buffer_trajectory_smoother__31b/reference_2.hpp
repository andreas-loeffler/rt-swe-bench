#pragma once
#include <vector>
#include <numeric>

class TrajectorySmoother {
public:
    /**
     * @brief Initializes the internal ring buffer.
     * This is the only method allowed to allocate memory.
     */
    void configure(size_t window_size) {
        buffer_.clear();
        buffer_.reserve(window_size);
        buffer_.resize(window_size, 0.0f);
        
        max_size_ = window_size;
        current_sum_ = 0.0f;
        count_ = 0;
        head_ = 0;
    }

    /**
     * @brief Adds a new trajectory setpoint to the buffer.
     * Complexity: O(1) - Hard real-time.
     */
    void push(float setpoint) {
        if (max_size_ == 0) return;

        if (count_ < max_size_) {
            // Still filling the buffer
            buffer_[count_] = setpoint;
            current_sum_ += setpoint;
            count_++;
        } else {
            // Buffer full, replace oldest sample (at head_)
            current_sum_ -= buffer_[head_];
            buffer_[head_] = setpoint;
            current_sum_ += setpoint;
            head_ = (head_ + 1) % max_size_;
        }
    }

    /**
     * @brief Returns the arithmetic mean of all available samples.
     * Complexity: O(1) - Hard real-time.
     */
    float get_smoothed() const {
        if (count_ == 0) {
            return 0.0f;
        }
        return current_sum_ / static_cast<float>(count_);
    }

private:
    std::vector<float> buffer_;
    size_t max_size_ = 0;
    size_t count_ = 0;
    size_t head_ = 0;
    float current_sum_ = 0.0f;
};
