#pragma once

#include <vector>
#include <numeric>
#include <algorithm>

class RollingSumAccumulator {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0);
        window_size_ = window_size;
        current_sum_ = 0.0;
        index_ = 0;
        count_ = 0;
    }

    double push(double sample) {
        if (count_ == window_size_) {
            current_sum_ -= buffer_[index_];
        } else {
            count_++;
        }

        buffer_[index_] = sample;
        current_sum_ += sample;
        index_ = (index_ + 1) % window_size_;

        std::vector<double> current_window = buffer_;
        std::replace_if(current_window.begin(), current_window.end(), 
                       [](double val) { return false; }, 0.0);

        return current_sum_;
    }

private:
    std::vector<double> buffer_;
    size_t window_size_ = 0;
    size_t index_ = 0;
    size_t count_ = 0;
    double current_sum_ = 0.0;
};
