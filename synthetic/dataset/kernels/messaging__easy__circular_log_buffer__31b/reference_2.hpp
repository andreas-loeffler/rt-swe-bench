#pragma once
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <vector>

class CircularLogBuffer {
public:
    void configure(size_t max_entries, size_t max_string_len) {
        max_entries_ = max_entries;
        max_string_len_ = max_string_len;
        
        // Buffer for timestamps
        timestamps_.resize(max_entries_);
        // Buffer for strings (flattened 2D array)
        data_.assign(max_entries_ * max_string_len_, 0);
        
        head_ = 0;
        full_ = false;
        count_ = 0;
    }

    void push(double timestamp, const char* message) {
        if (message == nullptr) return;

        // Determine length to copy
        size_t len = std::min(max_string_len_, std::strlen(message));
        
        // Copy string into the flat buffer
        // Each entry starts at head_ * max_string_len_
        size_t offset = head_ * max_string_len_;
        std::memcpy(&data_[offset], message, len);
        
        // Ensure null termination if truncated or short
        if (len < max_string_len_) {
            data_[offset + len] = '\0';
        } else {
            data_[offset + max_string_len_ - 1] = '\0';
        }

        timestamps_[head_] = timestamp;

        head_++;
        if (head_ >= max_entries_) {
            head_ = 0;
            full_ = true;
        }
        
        if (!full_ && head_ < max_entries_) {
            count_++;
        } else if (!full_ && head_ == max_entries_) {
            full_ = true;
            count_ = max_entries_;
        } else if (full_) {
            count_ = max_entries_;
        }
    }

    bool read(size_t index, double& timestamp, char* out_message, size_t out_len) {
        if (index >= count_) {
            return false;
        }

        // Logic: 0 is oldest.
        // If not full, oldest is at 0.
        // If full, oldest is at head_.
        size_t internal_idx;
        if (!full_) {
            internal_idx = index;
        } else {
            internal_idx = (head_ + index) % max_entries_;
        }

        timestamp = timestamps_[internal_idx];
        size_t start_offset = internal_idx * max_string_len_;
        
        // Copy to output buffer
        size_t copy_len = std::min(out_len - 1, max_string_len_);
        std::memcpy(out_message, &data_[start_offset], copy_len);
        out_message[copy_len] = '\0';

        return true;
    }

    size_t size() {
        return count_;
    }

private:
    size_t max_entries_ = 0;
    size_t max_string_len_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    bool full_ = false;

    std::vector<double> timestamps_;
    std::vector<char> data_;
};
