#pragma once

#include <cstddef>
#include <vector>

class FixedCapacityStack {
public:
    void configure(size_t capacity) {
        data_.reserve(capacity);
        data_.resize(capacity);
        capacity_ = capacity;
        top_ = 0;
    }

    bool push(int value) {
        if (top_ >= capacity_) {
            return false;
        }
        data_[top_] = value;
        top_++;
        return true;
    }

    bool pop(int& out_value) {
        if (top_ == 0) {
            return false;
        }
        top_--;
        out_value = data_[top_];
        return true;
    }

    size_t size() const {
        return top_;
    }

private:
    std::vector<int> data_;
    size_t capacity_ = 0;
    size_t top_ = 0;
};
