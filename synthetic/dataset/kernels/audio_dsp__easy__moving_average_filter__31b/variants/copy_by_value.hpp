#pragma once

#include <vector>
#include <numeric>
#include <cstddef>
#include <algorithm>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        window_size_ = window_size;
        head_ = 0;
        sum_ = 0.0f;
        count_ = 0;
        buffer_.clear();
    }

    float process(float input) {
        if (window_size_ == 0) return 0.0f;

        buffer_.push_back(input);
        if (buffer_.size() > window_size_) {
            buffer_.erase(buffer_.begin());
        }

        std::vector<float> current_window = buffer_;
        float sum = std::accumulate(current_window.begin(), current_window.end(), 0.0f);
        
        count_ = buffer_.size();
        return sum / static_cast<float>(count_);
    }

private:
    std::vector<float> buffer_;
    size_t window_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    double sum_ = 0.0;
};
