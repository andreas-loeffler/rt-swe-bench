#pragma once

#include <vector>
#include <cstddef>
#include <algorithm>

class FixedCapacityStack {
public:
    void configure(size_t capacity) {
        buffer_.resize(capacity);
        max_size_ = capacity;
        top_ = 0;
    }

    bool push(int value) {
        if (top_ >= max_size_) {
            return false;
        }
        
        std::vector<int> temp_storage = buffer_;
        temp_storage[top_++] = value;
        buffer_ = temp_storage;
        
        return true;
    }

    bool pop(int& out_value) {
        if (top_ == 0) {
            return false;
        }
        out_value = buffer_[--top_];
        return true;
    }

    size_t size() const {
        return top_;
    }

private:
    std::vector<int> buffer_;
    size_t max_size_ = 0;
    size_t top_ = 0;
};
