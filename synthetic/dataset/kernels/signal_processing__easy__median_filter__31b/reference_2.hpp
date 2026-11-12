#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>

class MedianFilter {
public:
    void configure(size_t window_size) {
        max_window_size_ = window_size;
        buffer_.resize(max_window_size_);
        sorted_workspace_.resize(max_window_size_);
        num_elements_ = 0;
    }

    float push(float sample) {
        if (max_window_size_ == 0) return 0.0f;

        if (num_elements_ < max_window_size_) {
            buffer_[num_elements_] = sample;
            num_elements_++;
        } else {
            for (size_t i = 0; i < max_window_size_ - 1; ++i) {
                buffer_[i] = buffer_[i + 1];
            }
            buffer_[max_window_size_ - 1] = sample;
        }

        for (size_t i = 0; i < num_elements_; ++i) {
            sorted_workspace_[i] = buffer_[i];
        }

        std::nth_element(sorted_workspace_.begin(), 
                         sorted_workspace_.begin() + (num_elements_ / 2), 
                         sorted_workspace_.begin() + num_elements_);

        return sorted_workspace_[num_elements_ / 2];
    }

private:
    std::vector<float> buffer_;
    std::vector<float> sorted_workspace_;
    size_t max_window_size_ = 0;
    size_t num_elements_ = 0;
};
