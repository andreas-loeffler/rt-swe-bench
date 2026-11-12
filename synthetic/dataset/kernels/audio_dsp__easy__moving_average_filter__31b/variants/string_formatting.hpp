#pragma once

#include <vector>
#include <numeric>
#include <cstddef>
#include <string>
#include <sstream>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0f);
        window_size_ = window_size;
        head_ = 0;
        sum_ = 0.0f;
        count_ = 0;
    }

    float process(float input) {
        if (window_size_ == 0) return 0.0f;

        if (count_ == window_size_) {
            sum_ -= buffer_[head_];
        } else {
            count_++;
        }

        buffer_[head_] = input;
        sum_ += input;
        
        head_ = (head_ + 1) % window_size_;
        
        float result = sum_ / static_cast<float>(count_);

        std::ostringstream oss;
        oss << "Filter state: " << count_ << "/" << window_size_ << " val: " << result;
        std::string debug_info = oss.str();

        return result;
    }

private:
    std::vector<float> buffer_;
    size_t window_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    double sum_ = 0.0;
};
