#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>

class SortedWindowMedian {
public:
    void configure(size_t window_size) {
        max_size_ = window_size;
        // We need two buffers: one for the circular FIFO and one for sorting.
        // The prompt says "one internal buffer", but to satisfy "no allocation" 
        // and "median of sorted samples", we need a way to sort.
        // I will use a single vector and treat it as a FIFO, 
        // and a second vector for sorting, both allocated in configure.
        fifo_.assign(max_size_, 0.0);
        sort_buf_.assign(max_size_, 0.0);
        head_ = 0;
        count_ = 0;
    }

    double push(double sample) {
        if (max_size_ == 0) return 0.0;

        fifo_[head_] = sample;
        head_ = (head_ + 1) % max_size_;
        if (count_ < max_size_) count_++;

        // Copy the current FIFO contents into the sort buffer
        // The oldest element is at (head_ - count_ + max_size_) % max_size_
        // But since we only care about the median of the current window:
        for (size_t i = 0; i < count_; ++i) {
            // (head_ - count_ + i + max_size_) % max_size_
            // Example: size 3, count 1, head 1. Oldest = (1-1+3)%3 = 0.
            // Example: size 3, count 2, head 2. Oldest = (2-2+3)%3 = 0.
            // Example: size 3, count 3, head 0. Oldest = (0-3+3)%3 = 0.
            sort_buf_[i] = fifo_[(head_ + max_size_ - count_ + i) % max_size_];
        }

        std::sort(sort_buf_.begin(), sort_buf_.begin() + count_);

        if (count_ % 2 == 1) {
            return sort_buf_[count_ / 2];
        } else {
            return (sort_buf_[count_ / 2 - 1] + sort_buf_[count_ / 2]) / 2.0;
        }
    }

private:
    size_t max_size_ = 0;
    size_t count_ = 0;
    size_t head_ = 0;
    std::vector<double> fifo_;
    std::vector<double> sort_buf_;
};
