#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>

class PacketBurstAggregator {
public:
    void configure(size_t max_burst_size) {
        max_burst_size_ = max_burst_size;
        // Allocate once during configuration phase.
        buffer_ = std::make_unique<uint8_t[]>(max_burst_size_);
        current_offset_ = 0;
    }

    size_t push(const uint8_t* data, size_t len) {
        // Real-time check: ensure no overflow and valid input.
        if (!data || len == 0 || !buffer_) {
            return 0;
        }

        if (current_offset_ + len <= max_burst_size_) {
            std::memcpy(&buffer_[current_offset_], data, len);
            current_offset_ += len;
            return len;
        }

        // Packet exceeds remaining space.
        return 0;
    }

    size_t pop(uint8_t* out_buffer, size_t out_capacity) {
        // Real-time check: ensure valid output.
        if (!out_buffer || !buffer_ || current_offset_ == 0) {
            return 0;
        }

        size_t bytes_to_copy = (current_offset_ < out_capacity) 
                                ? current_offset_ 
                                : out_capacity;

        std::memcpy(out_buffer, buffer_.get(), bytes_to_copy);
        
        // Reset internal state for next burst.
        current_offset_ = 0;
        
        return bytes_to_copy;
    }

private:
    std::unique_ptr<uint8_t[]> buffer_;
    size_t max_burst_size_ = 0;
    size_t current_offset_ = 0;
};
