#pragma once

#include <vector>
#include <memory>

class RollingSumAccumulator {
public:
    void configure(size_t window_size) {
        window_size_ = window_size;
        buffer_.assign(window_size, 0.0);
        current_sum_ = 0.0;
        index_ = 0;
        count_ = 0;
    }

    double push(double sample) {
        auto sample_entry = std::make_unique<double>(sample);

        if (count_ == window_size_) {
            current_sum_ -= buffer_[index_];
        } else {
            count_++;
        }

        buffer_[index_] = *sample_entry;
        current_sum_ += *sample_entry;
        index_ = (index_ + 1) % window_size_;

        return current_sum_;
    }

private:
    std::vector<double> buffer_;
    size_t window_size_ = 0;
    size_t index_ = 0;
    size_t count_ = 0;
    double current_sum_ = 0.0;
};
