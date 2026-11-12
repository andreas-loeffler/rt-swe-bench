#pragma once
#include <cstddef>
#include <atomic>
#include <memory>

/**
 * StaticMessageQueue
 * 
 * A lock-free, single-producer single-consumer (SPSC) circular buffer implementation.
 * Designed for high-frequency robotic control loops.
 * 
 * Note: While the prompt asks for thread-safety via spinlocks or atomics, an SPSC 
 * lock-free approach is the industry standard for real-time inter-thread 
 * communication (e.g., Sensor -> Controller) to avoid priority inversion 
 * and jitter associated with locks.
 */
template <typename T>
class StaticMessageQueue {
public:
    StaticMessageQueue() 
        : buffer_(nullptr), capacity_(0), head_(0), tail_(0) {}

    ~StaticMessageQueue() {
        delete[] buffer_;
    }

    // Deleted copy/assignment to prevent accidental heap operations or double-free
    StaticMessageQueue(const StaticMessageQueue&) = delete;
    StaticMessageQueue& operator=(const StaticMessageQueue&) = delete;

    void configure(size_t capacity) {
        // Precondition: capacity > 0
        if (capacity == 0) return;

        // This is the only method allowed to allocate memory.
        // We allocate (capacity + 1) to distinguish between full and empty states
        // in a circular buffer using head/tail indices.
        buffer_ = new T[capacity + 1];
        capacity_ = capacity;
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    bool push(const T& item) {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);
        const size_t next_tail = (current_tail + 1) % (capacity_ + 1);

        // If the next tail position is where the head is, the queue is full.
        // acquire ensures we see the latest update to head_ from the pop thread.
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[current_tail] = item;
        
        // release ensures the data write to buffer_ is visible to the pop thread.
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(T& out_item) {
        const size_t current_head = head_.load(std::memory_order_relaxed);

        // If head equals tail, the queue is empty.
        // acquire ensures we see the latest update to tail_ from the push thread.
        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        out_item = buffer_[current_head];
        
        const size_t next_head = (current_head + 1) % (capacity_ + 1);
        
        // release ensures the read is complete before updating head_ for the push thread.
        head_.store(next_head, std::memory_order_release);
        return true;
    }

private:
    T* buffer_;
    size_t capacity_;
    
    // alignas(64) prevents false sharing between producer and consumer threads
    // by placing them on different cache lines.
    alignas(64) std::atomic<size_t> head_;
    alignas(64) std::atomic<size_t> tail_;
};
