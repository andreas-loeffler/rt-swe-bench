#pragma once

#include <vector>
#include <numeric>
#include <algorithm>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0);
        window_size_ = window_size;
        index_ = 0;
        count_ = 0;
    }

    double process(double sample) {
        if (window_size_ == 0) return 0.0;

        if (count_ < window_size_) {
            count_++;
        }

        buffer_[index_] = sample;
        index_ = (index_ + 1) % window_size_;

        std::vector<double> current_window;
        current_window.reserve(count_);
        
        for (size_t i = 0; i < count_; ++i) {
            current_window.push_back(buffer_[i]);
        }

        double sum = std::accumulate(current_window.begin(), current_window.end(), 0.0);
        return sum / count_;
    }

private:
    std::vector<double> buffer_;
    size_t window_size_ = 0;
    size_t index_ = 0;
    size_t count_ = 0;
};
