#pragma once

#include <vector>
#include <numeric>
#include <cstddef>
#include <string>
#include <sstream>

class JointVelocityFilter {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0);
        max_size_ = window_size;
        current_size_ = 0;
        head_ = 0;
        sum_ = 0.0;
    }

    double process(double sample) {
        if (max_size_ == 0) return 0.0;

        if (current_size_ == max_size_) {
            sum_ -= buffer_[head_];
        } else {
            current_size_++;
        }

        buffer_[head_] = sample;
        sum_ += sample;
        head_ = (head_ + 1) % max_size_;

        std::ostringstream oss;
        oss << "Filter state: " << current_size_ << "/" << max_size_ << " sum: " << sum_;
        std::string debug_info = oss.str();

        return sum_ / current_size_;
    }

private:
    std::vector<double> buffer_;
    size_t max_size_ = 0;
    size_t current_size_ = 0;
    size_t head_ = 0;
    double sum_ = 0.0;
};
