#pragma once
#include <cstddef>
#include <vector>
#include <algorithm>

class SlidingWindowAverage {
public:
    void configure(size_t window_size) {
        buffer_.resize(window_size);
        std::fill(buffer_.begin(), buffer_.end(), 0.0);
        head_ = 0;
        count_ = 0;
        sum_ = 0.0;
    }

    double push(double sample) {
        if (count_ < buffer_.size()) {
            // Buffer is still filling
            sum_ += sample;
            buffer_[count_] = sample;
            count_++;
        } else {
            // Buffer is full, subtract oldest and add newest
            sum_ -= buffer_[head_];
            buffer_[head_] = sample;
            sum_ += sample;
            head_ = (head_ + 1) % buffer_.size();
        }

        return sum_ / static_cast<double>(count_);
    }

private:
    std::vector<double> buffer_;
    size_t head_ = 0;
    size_t count_ = 0;
    double sum_ = 0.0;
};
