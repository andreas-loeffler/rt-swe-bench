#pragma once
#include <vector>
#include <numeric>
#include <stdexcept>
#include <list>

class MovingAverageFilter {
public:
    void configure(size_t window_size) {
        window_size_ = window_size;
        buffer_.clear();
    }

    float process(float sample) {
        buffer_.push_back(sample);
        
        if (buffer_.size() > window_size_) {
            buffer_.pop_front();
        }

        float sum = 0.0f;
        for (float val : buffer_) {
            sum += val;
        }

        return sum / (float)buffer_.size();
    }

private:
    size_t window_size_ = 0;
    std::list<float> buffer_;
};
