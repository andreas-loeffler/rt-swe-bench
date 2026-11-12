#pragma once
#include <vector>
#include <atomic>
#include <cstddef>

template <typename T>
class FixedRingBuffer {
public:
    void configure(size_t capacity) {
        buffer_.resize(capacity);
        capacity_ = capacity;
        head_ = 0;
        tail_ = 0;
        count_ = 0;
    }

    bool push(const T& item) {
        size_t current_count = count_.load(std::memory_order_relaxed);
        if (current_count >= capacity_) {
            return false;
        }
        buffer_[head_] = item;
        head_ = (head_ + 1) % capacity_;
        count_.fetch_add(1, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        size_t current_count = count_.load(std::memory_order_acquire);
        if (current_count == 0) {
            return false;
        }
        item = buffer_[tail_];
        tail_ = (tail_ + 1) % capacity_;
        count_.fetch_sub(1, std::memory_order_release);
        return true;
    }

private:
    std::vector<T> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t tail_ = 0;
    std::atomic<size_t> count_{0};
};
