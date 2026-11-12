#pragma once
#include <vector>
#include <algorithm>

class MedianFilter {
public:
    void configure(size_t window_size) {
        buffer_.resize(window_size);
        sorted_buffer_.resize(window_size);
        max_size_ = window_size;
        head_ = 0;
        count_ = 0;
    }

    void push(float sample) {
        if (max_size_ == 0) return;

        buffer_[head_] = sample;
        head_ = (head_ + 1) % max_size_;
        if (count_ < max_size_) {
            count_++;
        }
    }

    float process() {
        if (count_ == 0) return 0.0f;

        // Copy current valid window to a scratchpad for sorting
        // Since we cannot allocate in RT, we use the pre-allocated sorted_buffer_
        for (size_t i = 0; i < count_; ++i) {
            sorted_buffer_[i] = buffer_[i];
        }

        // Use nth_element to find the median without full sort if possible, 
        // but for simplicity and deterministic behavior on small windows:
        std::sort(sorted_buffer_.begin(), sorted_buffer_.begin() + count_);

        if (count_ % 2 == 1) {
            return sorted_buffer_[count_ / 2];
        } else {
            return (sorted_buffer_[count_ / 2 - 1] + sorted_buffer_[count_ / 2]) * 0.5f;
        }
    }

private:
    std::vector<float> buffer_;
    std::vector<float> sorted_buffer_;
    size_t max_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
};
