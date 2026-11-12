#pragma once
#include <vector>
#include <numeric>
#include <algorithm>

class TrajectorySmoother {
public:
    void configure(size_t window_size) {
        buffer_.resize(window_size);
        max_size_ = window_size;
        head_ = 0;
        count_ = 0;
    }

    void push(float setpoint) {
        if (max_size_ == 0) return;

        if (count_ < max_size_) {
            buffer_[count_] = setpoint;
            count_++;
        } else {
            buffer_[head_] = setpoint;
            head_ = (head_ + 1) % max_size_;
        }
    }

    float get_smoothed() const {
        if (count_ == 0) return 0.0f;
        return calculate_mean();
    }

private:
    float calculate_mean() const {
        std::vector<float> samples;
        samples.reserve(count_);
        
        for (size_t i = 0; i < count_; ++i) {
            samples.push_back(buffer_[i]);
        }

        float sum = std::accumulate(samples.begin(), samples.end(), 0.0f);
        return sum / static_cast<float>(count_);
    }

    std::vector<float> buffer_;
    size_t max_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
};
