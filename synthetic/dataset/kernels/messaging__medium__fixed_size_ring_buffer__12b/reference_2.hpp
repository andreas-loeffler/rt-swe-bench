#pragma once
#include <vector>
#include <cstddef>
#include <atomic>

/**
 * FixedRingBuffer: A SPSC (Single-Producer Single-Consumer) lock-free circular buffer.
 * 
 * To ensure thread safety without mutexes, we use std::atomic for head and tail indices.
 * We use the "always leave one slot empty" strategy to distinguish between 
 * Full and Empty states without needing a separate counter.
 */
template <typename T>
class FixedRingBuffer {
public:
    void configure(size_t capacity) {
        // Reserve capacity + 1 to distinguish full from empty states
        // in a lock-free SPSC manner.
        capacity_ = capacity;
        buffer_.resize(capacity + 1);
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    bool push(const T& item) {
        // Producer reads head (where it writes) and tail (where consumer reads)
        const size_t current_head = head_.load(std::memory_order_relaxed);
        const size_t current_tail = tail_.load(std::memory_order_acquire);

        size_t next_head = (current_head + 1) % (capacity_ + 1);

        // If the next head position is where the tail is, the buffer is full
        if (next_head == current_tail) {
            return false;
        }

        buffer_[current_head] = item;
        
        // Release: ensures the write to buffer_ is visible to the consumer
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        // Consumer reads tail (where it reads) and head (where producer writes)
        const size_t current_tail = tail_.load(std::memory_order_relaxed);
        const size_t current_head = head_.load(std::memory_order_acquire);

        // If tail equals head, the buffer is empty
        if (current_tail == current_head) {
            return false;
        }

        item = buffer_[current_tail];

        size_t next_tail = (current_tail + 1) % (capacity_ + 1);
        
        // Release: ensures the read of buffer_ is complete before updating tail
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

private:
    std::vector<T> buffer_;
    size_t capacity_ = 0;
    
    // head_ is modified by Producer, read by Consumer
    std::atomic<size_t> head_{0};
    // tail_ is modified by Consumer, read by Producer
    std::atomic<size_t> tail_{0};
};
