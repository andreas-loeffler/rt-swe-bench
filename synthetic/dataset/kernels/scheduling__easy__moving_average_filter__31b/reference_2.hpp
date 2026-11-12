#pragma once
#include <cstddef>
#include <vector>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        buffer_.resize(window_size);
        window_size_ = window_size;
        head_ = 0;
        count_ = 0;
        sum_ = 0.0;
    }

    double process(double input) {
        if (count_ < window_size_) {
            sum_ += input;
            buffer_[count_] = input;
            count_++;
            return sum_ / count_;
        } else {
            // Steady state: subtract oldest, add newest
            sum_ -= buffer_[head_];
            buffer_[head_] = input;
            sum_ += input;
            head_ = (head_ + 1) % window_size_;
            return sum_ / window_size_;
        }
    }

private:
    std::vector<double> buffer_;
    size_t window_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    double sum_ = 0.0;
};
