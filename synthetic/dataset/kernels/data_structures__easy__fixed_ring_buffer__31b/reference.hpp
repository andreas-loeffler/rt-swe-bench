#pragma once

#include <vector>
#include <optional>

template <typename T>
class FixedRingBuffer {
public:
    void configure(size_t capacity) {
        buffer_.resize(capacity);
        capacity_ = capacity;
        head_ = 0;
        count_ = 0;
    }

    T push(T value) {
        T oldest = T{};
        if (count_ == capacity_) {
            oldest = buffer_[head_];
        }

        buffer_[head_] = value;
        head_ = (head_ + 1) % capacity_;
        if (count_ < capacity_) {
            count_++;
        }

        return oldest;
    }

private:
    std::vector<T> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
};
