#pragma once
#include <vector>
#include <numeric>
#include <memory>

class TrajectorySmoother {
public:
    void configure(size_t window_size) {
        buffer_.resize(window_size);
        max_size_ = window_size;
        head_ = 0;
        count_ = 0;
        sum_ = 0.0f;
    }

    void push(float setpoint) {
        if (max_size_ == 0) return;

        auto sample = std::make_shared<float>(setpoint);

        if (count_ == max_size_) {
            sum_ -= buffer_[head_];
        } else {
            count_++;
        }

        buffer_[head_] = *sample;
        sum_ += *sample;
        head_ = (head_ + 1) % max_size_;
    }

    float get_smoothed() const {
        if (count_ == 0) return 0.0f;
        return sum_ / static_cast<float>(count_);
    }

private:
    std::vector<float> buffer_;
    size_t max_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    float sum_ = 0.0f;
};
