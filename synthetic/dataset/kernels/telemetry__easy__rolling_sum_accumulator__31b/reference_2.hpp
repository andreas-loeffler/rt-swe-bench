#pragma once

#include <cstddef>
#include <vector>

class RollingSumAccumulator {
public:
    void configure(size_t window_size) {
        if (window_size == 0) return;
        
        buffer_.clear();
        buffer_.reserve(window_size);
        for (size_t i = 0; i < window_size; ++i) {
            buffer_.push_back(0.0);
        }
        
        max_size_ = window_size;
        current_sum_ = 0.0;
        head_ = 0;
        count_ = 0;
    }

    double push(double sample) {
        // No allocation, no locks, no exceptions.
        // Hard real-time path.
        
        if (count_ < max_size_) {
            buffer_[count_] = sample;
            current_sum_ += sample;
            count_++;
        } else {
            // Window is full, subtract oldest and add newest
            current_sum_ -= buffer_[head_];
            buffer_[head_] = sample;
            current_sum_ += sample;
            
            // Move head to the next oldest element (circular buffer)
            head_ = (head_ + 1) % max_size_;
        }
        
        return current_sum_;
    }

private:
    std::vector<double> buffer_;
    size_t max_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    double current_sum_ = 0.0;
};
