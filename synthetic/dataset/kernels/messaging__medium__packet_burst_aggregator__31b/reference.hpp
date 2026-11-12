#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cstring>

class PacketBurstAggregator {
public:
    void configure(size_t max_burst_size) {
        buffer_.resize(max_burst_size);
        capacity_ = max_burst_size;
        offset_ = 0;
    }

    size_t push(const uint8_t* data, size_t len) {
        if (data == nullptr || len == 0) return 0;
        if (offset_ + len > capacity_) {
            return 0;
        }
        std::memcpy(&buffer_[offset_], data, len);
        offset_ += len;
        return len;
    }

    size_t pop(uint8_t* out_buffer, size_t out_capacity) {
        if (out_buffer == nullptr || out_capacity == 0) return 0;
        size_t bytes_to_copy = std::min(offset_, out_capacity);
        std::memcpy(out_buffer, buffer_.data(), bytes_to_copy);
        offset_ = 0;
        return bytes_to_copy;
    }

private:
    std::vector<uint8_t> buffer_;
    size_t capacity_ = 0;
    size_t offset_ = 0;
};
