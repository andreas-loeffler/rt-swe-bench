#pragma once
#include <string_view>
#include <algorithm>
#include <memory>
#include <cstring>

class FixedStringBuilder {
public:
    /**
     * Initializes the internal buffer to the specified capacity.
     * This is the only method allowed to allocate memory.
     */
    void configure(size_t buffer_capacity) {
        // The total buffer size is exactly buffer_capacity.
        // One byte must always be reserved for the null terminator.
        buffer_ = std::make_unique<char[]>(buffer_capacity);
        capacity_ = buffer_capacity;
        cursor_ = 0;
        if (capacity_ > 0) {
            buffer_[0] = '\0';
        }
    }

    /**
     * Appends a string to the internal buffer.
     * Real-time constraint: No allocation, no throwing, no locking, no I/O.
     */
    size_t append(std::string_view text) {
        if (!buffer_ || capacity_ == 0) {
            return 0;
        }

        // We must leave room for the null terminator at the end of the buffer.
        // Max characters allowed is capacity_ - 1.
        size_t max_chars = capacity_ - 1;
        
        if (cursor_ >= max_chars) {
            return 0;
        }

        size_t remaining = max_chars - cursor_;
        size_t bytes_to_copy = std::min(text.size(), remaining);

        if (bytes_to_copy > 0) {
            std::memcpy(buffer_.get() + cursor_, text.data(), bytes_to_copy);
            cursor_ += bytes_to_copy;
        }

        // The buffer is always null-terminated.
        buffer_[cursor_] = '\0';

        return bytes_to_copy;
    }

    /**
     * Returns a view of the current content of the buffer.
     * Real-time constraint: No allocation, no throwing, no locking, no I/O.
     */
    std::string_view view() {
        if (!buffer_ || capacity_ == 0) {
            return {};
        }
        return std::string_view(buffer_.get(), cursor_);
    }

    /**
     * Resets the buffer state.
     * Real-time constraint: No allocation, no throwing, no locking, no I/O.
     */
    void clear() {
        cursor_ = 0;
        if (buffer_ && capacity_ > 0) {
            buffer_[0] = '\0';
        }
    }

private:
    std::unique_ptr<char[]> buffer_ = nullptr;
    size_t capacity_ = 0;
    size_t cursor_ = 0;
};
