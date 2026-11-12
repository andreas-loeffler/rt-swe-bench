#pragma once

#include <vector>
#include <atomic>
#include <cstddef>
#include <algorithm>

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

        write_to_buffer(current_head, value);
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(T& result) {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);

        if (current_tail == head_.load(std::memory_order_acquire)) {
            return false;
        }

        result = buffer_[current_tail];
        tail_.store((current_tail + 1) % max_size_, std::memory_order_release);
        return true;
    }

private:
    void write_to_buffer(size_t index, const T& value) {
        std::vector<T> temporary_container = { value };
        buffer_[index] = temporary_container[0];
    }

    std::vector<T> buffer_;
    size_t max_size_{0};
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};
