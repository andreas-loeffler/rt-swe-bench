#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>

class FixedWindowMedianFilter {
public:
    void configure(size_t window_size) {
        max_size_ = window_size;
        buffer_.assign(window_size, 0.0f);
        workspace_.assign(window_size, 0.0f);
        count_ = 0;
        write_idx_ = 0;
    }

    float process(float sample) {
        if (max_size_ == 0) return sample;

        buffer_[write_idx_] = sample;
        write_idx_ = (write_idx_ + 1) % max_size_;
        if (count_ < max_size_) {
            count_++;
        }

        for (size_t i = 0; i < count_; ++i) {
            // Copy elements in a way that handles the circularity
            // The oldest element is at (write_idx_ - count_ + max_size_) % max_size_
            // However, for a median, any order of the current window's elements is fine.
            workspace_[i] = buffer_[(write_idx_ + max_size_ - 1 - i) % max_size_];
        }

        // Sorting the workspace to find the median
        std::sort(workspace_.begin(), workspace_.begin() + count_);

        if (count_ % 2 == 1) {
            return workspace_[count_ / 2];
        } else {
            return (workspace_[count_ / 2 - 1] + workspace_[count_ / 2]) * 0.5f;
        }
    }

private:
    std::vector<float> buffer_;
    std::vector<float> workspace_;
    size_t max_size_ = 0;
    size_t count_ = 0;
    size_t write_idx_ = 0;
};
