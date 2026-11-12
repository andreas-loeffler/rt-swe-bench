#pragma once
#include <vector>
#include <cstddef>

class RingBufferLogger {
public:
    void configure(size_t capacity) {
        buffer_.resize(capacity);
        capacity_ = capacity;
        head_ = 0;
        count_ = 0;
    }

    void push(float sample) {
        buffer_[head_] = sample;
        head_ = (head_ + 1) % capacity_;
        if (count_ < capacity_) {
            count_++;
        }
    }

    size_t size() const {
        return count_;
    }

    float get(size_t index) const {
        // index 0 is oldest. 
        // If not full: oldest is at 0.
        // If full: oldest is at head_.
        size_t start = (count_ < capacity_) ? 0 : head_;
        size_t actual_idx = (start + index) % capacity_;
        return buffer_[actual_idx];
    }

private:
    std::vector<float> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
};
