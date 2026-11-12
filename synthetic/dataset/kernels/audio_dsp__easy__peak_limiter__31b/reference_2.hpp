#pragma once
#include <cstddef>
#include <vector>
#include <cmath>
#include <algorithm>

class PeakLimiter {
public:
    void configure(float threshold, size_t lookahead_samples) {
        threshold_ = threshold;
        lookahead_count_ = lookahead_samples;
        buffer_.assign(lookahead_count_, 0.0f);
        write_index_ = 0;
    }

    float process(float input) {
        // To achieve a delay of N with a buffer of size N:
        // 1. The output is the sample at the current write position (the oldest sample).
        // 2. The current input is written into that same position.
        // 3. The write pointer moves forward.
        
        float sample = buffer_[write_index_];
        buffer_[write_index_] = input;
        write_index_ = (write_index_ + 1) % lookahead_count_;

        if (std::abs(sample) > threshold_) {
            return (sample > 0.0f) ? threshold_ : -threshold_;
        }
        return sample;
    }

private:
    float threshold_ = 0.0f;
    size_t lookahead_count_ = 0;
    size_t write_index_ = 0;
    std::vector<float> buffer_;
};
