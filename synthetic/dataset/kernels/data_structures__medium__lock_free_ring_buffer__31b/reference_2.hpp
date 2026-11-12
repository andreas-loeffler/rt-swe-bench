#pragma once

#include <cstddef>
#include <memory>
#include <atomic>
#include <vector>

template <typename T>
class LockFreeRingBuffer {
public:
    LockFreeRingBuffer() : buffer_(nullptr), capacity_(0), head_(0), tail_(0) {}

    void configure(size_t capacity) {
        // Capacity is size - 1 to distinguish between full and empty states
        // using (tail + 1) % capacity == head. 
        // However, a common robust way is to allocate capacity + 1.
        size_t internal_capacity = capacity + 1;
        buffer_ = std::make_unique<T[]>(internal_capacity);
        capacity_ = internal_capacity;
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    bool push(T value) {
        if (!buffer_) return false;

        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) % capacity_;

        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }

        buffer_[current_tail] = std::move(value);
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(T& result) {
        if (!buffer_) return false;

        size_t current_head = head_.load(std::memory_order_relaxed);

        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false; // Buffer empty
        }

        result = std::move(buffer_[current_head]);
        head_.store((current_head + 1) % capacity_, std::memory_order_release);
        return true;
    }

private:
    std::unique_ptr<T[]> buffer_;
    size_t capacity_;
    // Using std::atomic to ensure thread safety between producer and consumer
    std::atomic<size_t> head_; // Read index
    std::atomic<size_t> tail_; // Write index
};
