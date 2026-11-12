#pragma once
#include <cstddef>
#include <vector>
#include <algorithm>

class MedianFilter {
public:
    void configure(size_t window_size) {
        window_size_ = window_size;
        buffer_.assign(window_size_, 0.0f);
        sorted_buffer_.assign(window_size_, 0.0f);
        head_ = 0;
        count_ = 0;
    }

    void push(float sample) {
        if (window_size_ == 0) return;

        buffer_[head_] = sample;
        head_ = (head_ + 1) % window_size_;
        if (count_ < window_size_) {
            count_++;
        }
    }

    float get_median() const {
        if (count_ == 0) return 0.0f;

        // We need to copy the buffer to sorted_buffer_ to sort it.
        // Because get_median is const, we cannot modify sorted_buffer_ directly
        // unless we make it mutable or use a workaround. 
        // However, the requirements say we must not allocate.
        // To fix the compilation error (assignment to read-only location), 
        // we mark sorted_buffer_ as mutable because it's an internal cache 
        // for the median calculation, not part of the observable state.
        
        for (size_t i = 0; i < count_; ++i) {
            // Correct index calculation for FIFO: 
            // The 'oldest' element is at (head_ - count_ + window_size_) % window_size_
            size_t idx = (head_ + window_size_ - count_ + i) % window_size_;
            sorted_buffer_[i] = buffer_[idx];
        }

        std::sort(sorted_buffer_.begin(), sorted_buffer_.begin() + count_);

        if (count_ % 2 == 1) {
            return sorted_buffer_[count_ / 2];
        } else {
            return (sorted_buffer_[count_ / 2 - 1] + sorted_buffer_[count_ / 2]) * 0.5f;
        }
    }

private:
    size_t window_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    std::vector<float> buffer_;
    mutable std::vector<float> sorted_buffer_;
};
