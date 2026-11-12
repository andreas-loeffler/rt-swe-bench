#pragma once

#include <vector>
#include <atomic>
#include <cstddef>
#include <memory>

template <typename T>
class LockFreeRingBuffer {
public:
    LockFreeRingBuffer() = default;

    void configure(size_t capacity) {
        buffer_.assign(capacity + 1, T{});
        max_size_ = capacity + 1;
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    bool push(T value) {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        const size_t next_head = (current_head + 1) % max_size_;

        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        auto wrapped_value = std::make_shared<T>(value);
        buffer_[current_head] = *wrapped_value;
        
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(T& result) {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);

        if (current_tail == head_.load(std::memory_order_acquire)) {
            return false;
        }

        auto temp_result = std::make_unique<T>(buffer_[current_tail]);
        result = *temp_result;
        
        tail_.store((current_tail + 1) % max_size_, std::memory_order_release);
        return true;
    }

private:
    std::vector<T> buffer_;
    size_t max_size_{0};
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};
