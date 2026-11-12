#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

class SlidingWindowChecksum {
public:
    void configure(size_t window_size) {
        window_size_ = window_size;
        // Use a vector to allocate memory outside the real-time path.
        buffer_.assign(window_size_, 0);
        current_checksum_ = 0;
        head_ = 0;
        count_ = 0;
    }

    void push(uint8_t byte) {
        // Logic for rolling sum:
        // If the buffer is full, subtract the byte being overwritten.
        if (count_ == window_size_) {
            current_checksum_ -= buffer_[head_];
        } else {
            count_++;
        }

        // Add new byte to buffer and sum
        buffer_[head_] = byte;
        current_checksum_ += byte;

        // Move head pointer circularly
        head_ = (head_ + 1) % window_size_;
    }

    uint64_t current_checksum() const {
        return current_checksum_;
    }

    size_t current_window_fill() const {
        return count_;
    }

private:
    size_t window_size_ = 0;
    std::vector<uint8_t> buffer_;
    uint64_t current_checksum_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
};
