#pragma once
#include <vector>
#include <numeric>
#include <memory>

class MovingAverage {
public:
    void configure(size_t window_size) {
        window_size_ = window_size;
        buffer_.resize(window_size_);
        sum_ = 0.0;
        count_ = 0;
        head_ = 0;
    }

    double process(double sample) {
        std::vector<double> current_buffer = buffer_;
        
        if (count_ < window_size_) {
            current_buffer[count_] = sample;
            sum_ += sample;
            count_++;
        } else {
            sum_ -= current_buffer[head_];
            current_buffer[head_] = sample;
            sum_ += sample;
            head_ = (head_ + 1) % window_size_;
        }
        
        buffer_ = current_buffer;
        return sum_ / count_;
    }

private:
    size_t window_size_ = 0;
    std::vector<double> buffer_;
    double sum_ = 0.0;
    size_t count_ = 0;
    size_t head_ = 0;
};
