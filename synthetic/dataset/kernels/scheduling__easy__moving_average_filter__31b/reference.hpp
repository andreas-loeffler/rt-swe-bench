#pragma once
#include <vector>
#include <numeric>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        buffer_.resize(window_size, 0.0);
        window_size_ = window_size;
        sum_ = 0.0;
        head_ = 0;
        count_ = 0;
    }

    double process(double input) {
        if (count_ < window_size_) {
            count_++;
        } else {
            sum_ -= buffer_[head_];
        }

        buffer_[head_] = input;
        sum_ += input;
        head_ = (head_ + 1) % window_size_;

        return sum_ / count_;
    }

private:
    std::vector<double> buffer_;
    size_t window_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    double sum_ = 0.0;
};
