#pragma once

#include <vector>
#include <deque>

template <typename T>
class FixedRingBuffer {
public:
    void configure(size_t capacity) {
        capacity_ = capacity;
        buffer_.clear();
        head_ = 0;
    }

    T push(T value) {
        T oldest = T{};
        if (buffer_.size() == capacity_) {
            oldest = buffer_[head_];
            buffer_[head_] = value;
            head_ = (head_ + 1) % capacity_;
        } else {
            buffer_.push_back(value);
        }

        return oldest;
    }

private:
    std::vector<T> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
};
