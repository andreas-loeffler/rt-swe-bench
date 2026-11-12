#pragma once
#include <cstddef>
#include <vector>

class LookAheadBuffer {
public:
    /**
     * @brief Configures the buffer capacity and performs memory allocation.
     * This is not a real-time method.
     */
    void configure(size_t capacity) {
        buffer_.clear();
        buffer_.reserve(capacity);
        capacity_ = capacity;
        head_ = 0;
        tail_ = 0;
        size_ = 0;
    }

    /**
     * @brief Adds a waypoint to the end of the FIFO queue.
     * Real-time safe: No allocation, no locking, no I/O.
     */
    bool push(double waypoint) {
        if (size_ == capacity_) {
            return false;
        }

        buffer_[tail_] = waypoint;
        tail_ = (tail_ + 1) % capacity_;
        size_++;
        return true;
    }

    /**
     * @brief Retrieves the oldest waypoint from the queue.
     * Real-time safe: No allocation, no locking, no I/O.
     */
    bool pop(double& out_waypoint) {
        if (size_ == 0) {
            return false;
        }

        out_waypoint = buffer_[head_];
        head_ = (head_ + 1) % capacity_;
        size_--;
        return true;
    }

private:
    std::vector<double> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
};
