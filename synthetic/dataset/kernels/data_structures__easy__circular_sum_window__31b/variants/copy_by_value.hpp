#pragma once

#include <vector>
#include <cstddef>
#include <numeric>

class CircularSumWindow {
public:
    void configure(size_t window_size) {
        max_size_ = window_size;
        buffer_.clear();
        buffer_.reserve(window_size);
        current_sum_ = 0.0;
        write_pos_ = 0;
        count_ = 0;
    }

    double push(double sample) {
        if (max_size_ == 0) return 0.0;

        if (count_ < max_size_) {
            buffer_.push_back(sample);
            count_++;
        } else {
            buffer_[write_pos_] = sample;
            write_pos_ = (write_pos_ + 1) % max_size_;
        }

        std::vector<double> current_window = buffer_;
        double sum = std::accumulate(current_window.begin(), current_window.end(), 0.0);

        return sum;
    }

private:
    std::vector<double> buffer_;
    size_t max_size_ = 0;
    size_t write_pos_ = 0;
    size_t count_ = 0;
    double current_sum_ = 0.0;
};
