#pragma once
#include <vector>
#include <algorithm>
#include <stdexcept>

class CircularBufferStats {
public:
    void configure(size_t capacity) {
        buffer_.resize(capacity);
        capacity_ = capacity;
        head_ = 0;
        full_ = false;
        count_ = 0;
        sorted_buffer_.resize(capacity);
    }

    double process(double value) {
        if (count_ < capacity_) {
            buffer_[count_] = value;
            count_++;
            if (count_ == capacity_) full_ = true;
        } else {
            buffer_[head_] = value;
            head_ = (head_ + 1) % capacity_;
        }

        // Copy elements to a pre-allocated buffer to avoid heap allocation in real-time
        for (size_t i = 0; i < count_; ++i) {
            sorted_buffer_[i] = buffer_[i];
        }

        // Use std::sort on the pre-allocated buffer segment
        std::sort(sorted_buffer_.begin(), sorted_buffer_.begin() + count_);

        if (count_ % 2 == 1) {
            return sorted_buffer_[count_ / 2];
        } else {
            return (sorted_buffer_[count_ / 2 - 1] + sorted_buffer_[count_ / 2]) / 2.0;
        }
    }

private:
    std::vector<double> buffer_;
    std::vector<double> sorted_buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    bool full_ = false;
};
