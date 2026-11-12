#pragma once

#include <cstdint>
#include <vector>
#include <atomic>
#include <memory>

/**
 * FixedCapacityCommandQueue
 * 
 * A lock-free SPSC (Single-Producer Single-Consumer) ring buffer.
 * Optimized for real-time use: no allocations, locks, or I/O in push/pop.
 * Uses std::atomic with acquire/release semantics for memory ordering.
 */
class FixedCapacityCommandQueue {
public:
    void configure(size_t capacity) {
        // To allow pushing 'capacity' elements, the underlying buffer must have 
        // a size of 'capacity + 1' if using the "one slot empty" ring buffer 
        // logic, or we must handle the wrap-around logic such that we never 
        // overwrite the head.
        
        // The test "capacity_is_exactly_16" suggests that if capacity is 16, 
        // the user expects 16 slots of usable space.
        size_t actual_capacity = (capacity > 0) ? capacity : 1;
        
        // We allocate capacity + 1 to distinguish between "full" and "empty"
        // while allowing exactly 'capacity' items to be stored.
        data.resize(actual_capacity + 1);
        m_capacity = actual_capacity + 1;
        
        // Initialize atomics
        head.store(0, std::memory_order_relaxed);
        tail.store(0, std::memory_order_relaxed);
    }

    bool push(uint32_t id, float value) {
        // Real-time path: No allocation, no locks.
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) % m_capacity;

        // If next_tail matches head, the buffer is full.
        if (next_tail == head.load(std::memory_order_acquire)) {
            return false;
        }

        data[current_tail] = {id, value};
        // Release ensures the data write is visible to the consumer before the tail update.
        tail.store(next_tail, std::memory_order_release);
        return true;
    }

    bool pop(uint32_t& id, float& value) {
        // Real-time path: No allocation, no locks.
        size_t current_head = head.load(std::memory_order_relaxed);

        // If head matches tail, the buffer is empty.
        if (current_head == tail.load(std::memory_order_acquire)) {
            return false;
        }

        id = data[current_head].id;
        value = data[current_head].value;

        // Release ensures the consumer is done reading before the head update is visible.
        head.store((current_head + 1) % m_capacity, std::memory_order_release);
        return true;
    }

private:
    struct Command {
        uint32_t id;
        float value;
    };

    std::vector<Command> data;
    size_t m_capacity = 0;

    // Align to cache lines to prevent false sharing in SPSC context
    alignas(64) std::atomic<size_t> head{0};
    alignas(64) std::atomic<size_t> tail{0};
};
