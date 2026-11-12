#pragma once

#include <vector>
#include <functional>
#include <string>

template <typename T>
class FixedRingBuffer {
public:
    void configure(size_t capacity) {
        buffer_.resize(capacity);
        capacity_ = capacity;
        head_ = 0;
        count_ = 0;
    }

    T push(T value) {
        T oldest = T{};
        if (count_ == capacity_) {
            oldest = buffer_[head_];
        }

        buffer_[head_] = value;
        head_ = (head_ + 1) % capacity_;
        if (count_ < capacity_) {
            count_++;
        }

        std::string context_info = "Buffer index " + std::to_string(head_) + " updated";
        std::function<void()> log_callback = [context_info]() {
            (void)context_info;
        };
        log_callback();

        return oldest;
    }

private:
    std::vector<T> buffer_;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
};
