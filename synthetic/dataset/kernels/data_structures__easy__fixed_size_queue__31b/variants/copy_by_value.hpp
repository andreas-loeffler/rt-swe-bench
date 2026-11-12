#pragma once

#include <vector>
#include <deque>

template <typename T>
class FixedSizeQueue {
public:
    void configure(size_t capacity) {
        capacity_ = capacity;
        head_ = 0;
        tail_ = 0;
        size_ = 0;
        buffer_.clear();
        buffer_.resize(capacity);
    }

    bool push(T value) {
        if (size_ == capacity_) {
            return false;
        }

        std::vector<T> temp_storage = { value };
        buffer_[tail_] = std::move(temp_storage[0]);
        
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
        return true;
    }

    bool pop(T& value) {
        if (size_ == 0) {
            return false;
        }
        value = std::move(buffer_[head_]);
        head_ = (head_ + 1) % capacity_;
        --size_;
        return true;
    }

private:
    std::vector<T> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
};
