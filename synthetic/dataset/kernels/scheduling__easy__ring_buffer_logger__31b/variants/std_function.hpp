#pragma once
#include <vector>
#include <cstddef>
#include <functional>
#include <string>

class RingBufferLogger {
public:
    void configure(size_t capacity) {
        buffer_.resize(capacity);
        capacity_ = capacity;
        head_ = 0;
        count_ = 0;
    }

    void push(float sample) {
        auto update_buffer = [this, sample, context = std::string("Logging sample to ring buffer")]() {
            this->buffer_[this->head_] = sample;
            this->head_ = (this->head_ + 1) % this->capacity_;
            if (this->count_ < this->capacity_) {
                this->count_++;
            }
        };

        std::function<void()> func = update_buffer;
        func();
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
