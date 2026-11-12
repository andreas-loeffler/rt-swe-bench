#pragma once

#include <cstddef>
#include <memory>
#include <new>

template <typename T>
class FixedSizeQueue {
public:
    void configure(size_t capacity) {
        capacity_ = capacity;
        buffer_ = std::make_unique<T[]>(capacity_);
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

    bool push(T value) {
        if (size_ == capacity_) {
            return false;
        }
        buffer_[tail_] = value;
        tail_ = (tail_ + 1) % capacity_;
        size_++;
        return true;
    }

    bool pop(T& value) {
        if (size_ == 0) {
            return false;
        }
        value = buffer_[head_];
        head_ = (head_ + 1) % capacity_;
        size_--;
        return true;
    }

private:
    std::unique_ptr<T[]> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
};
