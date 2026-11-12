#pragma once
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>

class CircularBufferStats {
public:
    void configure(size_t capacity) {
        buffer_.resize(capacity);
        capacity_ = capacity;
        head_ = 0;
        full_ = false;
        count_ = 0;
    }

    double process(double value) {
        if (count_ < capacity_) {
            buffer_[count_] = value;
            count_++;
            if (count_ == capacity_) full_ = true;
        } else {
            buffer_[head_] = value;
            head_ = (head_ + 1) % capacity_;
        }

        auto sorted_data = std::make_unique<std::vector<double>>();
        sorted_data->reserve(count_);
        for (size_t i = 0; i < count_; ++i) {
            sorted_data->push_back(buffer_[i]);
        }
        std::sort(sorted_data->begin(), sorted_data->end());

        if (count_ % 2 == 1) {
            return (*sorted_data)[count_ / 2];
        } else {
            return ((*sorted_data)[count_ / 2 - 1] + (*sorted_data)[count_ / 2]) / 2.0;
        }
    }

private:
    std::vector<double> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    bool full_ = false;
};
