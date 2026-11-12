#pragma once

#include <vector>
#include <cstddef>

class CircularSumWindow {
public:
    void configure(size_t window_size) {
        buffer_.assign(window_size, 0.0);
        max_size_ = window_size;
        current_sum_ = 0.0;
        write_pos_ = 0;
        count_ = 0;
    }

    double push(double sample) {
        if (max_size_ == 0) return 0.0;

        if (count_ == max_size_) {
            current_sum_ -= buffer_[write_pos_];
        } else {
            count_++;
        }

        buffer_[write_pos_] = sample;
        current_sum_ += sample;
        write_pos_ = (write_pos_ + 1) % max_size_;

        return current_sum_;
    }

private:
    std::vector<double> buffer_;
    size_t max_size_ = 0;
    size_t write_pos_ = 0;
    size_t count_ = 0;
    double current_sum_ = 0.0;
};
