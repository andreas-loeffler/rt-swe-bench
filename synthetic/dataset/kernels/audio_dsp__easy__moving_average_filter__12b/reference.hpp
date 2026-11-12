#pragma once
#include <vector>
#include <stdexcept>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        window_size_ = window_size;
        buffer_.assign(window_size, 0.0f);
        head_ = 0;
        count_ = 0;
    }

    float process(float sample) {
        if (count_ < window_size_) {
            buffer_[count_] = sample;
            count_++;
            sum_ += sample;
        } else {
            sum_ -= buffer_[head_];
            buffer_[head_] = sample;
            sum_ += sample;
            head_ = (head_ + 1) % window_size_;
        }
        
        return sum_ / (float)count_;
    }

private:
    size_t window_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    float sum_ = 0.0f;
    std::vector<float> buffer_;
};
