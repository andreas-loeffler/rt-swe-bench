#pragma once

#include <vector>
#include <numeric>
#include <algorithm>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        window_size_ = window_size;
        buffer_.clear();
        sum_ = 0.0;
        index_ = 0;
        count_ = 0;
    }

    double process(double sample) {
        if (window_size_ == 0) return 0.0;

        buffer_.push_back(sample);
        
        if (buffer_.size() > window_size_) {
            std::vector<double> window(buffer_.end() - window_size_, buffer_.end());
            buffer_ = window;
        }

        double current_sum = std::accumulate(buffer_.begin(), buffer_.end(), 0.0);
        return current_sum / buffer_.size();
    }

private:
    std::vector<double> buffer_;
    size_t window_size_ = 0;
    size_t index_ = 0;
    size_t count_ = 0;
    double sum_ = 0.0;
};
