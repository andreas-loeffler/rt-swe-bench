#pragma once
#include <vector>
#include <memory>
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
        auto sample_ptr = std::make_shared<float>(sample);
        buffer_[head_] = *sample_ptr;
        head_ = (head_ + 1) % capacity_;
        if (count_ < capacity_) {
            count_++;
        }
    }

    size_t size() const {
        return count_;
    }

    float get(size_t index) const {
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
