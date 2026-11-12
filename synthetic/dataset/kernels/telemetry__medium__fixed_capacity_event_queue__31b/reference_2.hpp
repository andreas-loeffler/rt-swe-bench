#pragma once

#include <vector>
#include <atomic>
#include <cstddef>

struct TelemetryEvent {
    uint32_t id;
    double timestamp;
    float value;

    bool operator==(const TelemetryEvent&) const = default;
};

/**
 * FixedCapacityEventQueue is a lock-free, SPSC ring buffer.
 * 
 * Fix Explanation:
 * The original implementation used `(current_tail + 1) % capacity_`.
 * In a ring buffer where `head == tail` represents "empty", a buffer of size N 
 * can only hold N-1 elements to distinguish the "full" state from "empty".
 * To support a capacity of N, the underlying storage must be size N+1 or 
 * the logic must account for the lost slot.
 * 
 * To strictly honor the `configure(capacity)` requirement where the buffer 
 * "holds up to capacity events", we allocate `capacity + 1` slots internally.
 */
class FixedCapacityEventQueue {
public:
    void configure(size_t capacity) {
        capacity_ = capacity;
        // Allocate capacity + 1 to allow for the 'empty' vs 'full' distinction
        buffer_.resize(capacity_ + 1);
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    bool push(const TelemetryEvent& event) {
        if (capacity_ == 0) return false;

        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) % (capacity_ + 1);

        // If next_tail reaches head, the buffer is full.
        // Note: head_ is loaded with acquire to see the latest pop.
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[current_tail] = event;
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(TelemetryEvent& event) {
        if (capacity_ == 0) return false;

        size_t current_head = head_.load(std::memory_order_relaxed);
        
        // If head equals tail, the buffer is empty.
        // Note: tail_ is loaded with acquire to see the latest push.
        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        event = buffer_[current_head];
        head_.store((current_head + 1) % (capacity_ + 1), std::memory_order_release);
        return true;
    }

private:
    std::vector<TelemetryEvent> buffer_;
    size_t capacity_ = 0;
    
    // Using indices into the (capacity_ + 1) sized buffer
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};
