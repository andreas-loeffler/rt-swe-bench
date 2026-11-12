#pragma once
#include <cstddef>
#include <vector>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        buffer_.clear();
        buffer_.resize(window_size);
        capacity_ = window_size;
        head_ = 0;
        count_ = 0;
        sum_ = 0.0;
    }

    double process(double sample) {
        if (capacity_ == 0) return 0.0;

        if (count_ < capacity_) {
            // Filling phase
            sum_ += sample;
            buffer_[count_] = sample;
            count_++;
        } else {
            // Sliding window phase
            sum_ -= buffer_[head_];
            buffer_[head_] = sample;
            sum_ += sample;
            head_ = (head_ + 1) % capacity_;
        }

        return sum_ / static_cast<double>(count_);
    }

private:
    std::vector<double> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    double sum_ = 0.0;
};
