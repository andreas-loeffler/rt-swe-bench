#pragma once

#include <vector>
#include <numeric>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0);
        window_size_ = window_size;
        sum_ = 0.0;
        index_ = 0;
        count_ = 0;
    }

    double process(double sample) {
        if (window_size_ == 0) return 0.0;

        if (count_ < window_size_) {
            count_++;
        } else {
            sum_ -= buffer_[index_];
        }

        buffer_[index_] = sample;
        sum_ += sample;
        index_ = (index_ + 1) % window_size_;

        return sum_ / count_;
    }

private:
    std::vector<double> buffer_;
    size_t window_size_ = 0;
    size_t index_ = 0;
    size_t count_ = 0;
    double sum_ = 0.0;
};
