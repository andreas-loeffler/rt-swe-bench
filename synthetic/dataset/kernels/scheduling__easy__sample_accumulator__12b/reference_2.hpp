#pragma once
#include <cstddef>
#include <vector>

class SampleAccumulator {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0);
        sum_ = 0.0;
        index_ = 0;
        count_ = 0;
        capacity_ = window_size;
    }

    double process(double sample) {
        if (capacity_ == 0) {
            return 0.0;
        }

        if (count_ < capacity_) {
            // Warm-up phase: filling the buffer
            buffer_[index_] = sample;
            sum_ += sample;
            count_++;
            index_ = (index_ + 1) % capacity_;
        } else {
            // Steady state: sliding window
            // Subtract the oldest sample (which is at the current index)
            sum_ -= buffer_[index_];
            // Add the new sample
            buffer_[index_] = sample;
            sum_ += sample;
            // Advance the index
            index_ = (index_ + 1) % capacity_;
        }

        return sum_;
    }

private:
    std::vector<double> buffer_;
    double sum_ = 0.0;
    size_t index_ = 0;
    size_t count_ = 0;
    size_t capacity_ = 0;
};
