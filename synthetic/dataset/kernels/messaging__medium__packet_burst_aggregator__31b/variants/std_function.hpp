#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cstring>
#include <functional>

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

        execute_copy(data, len);
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
    void execute_copy(const uint8_t* src, size_t len) {
        struct CopyContext {
            const uint8_t* src_ptr;
            size_t length;
            uint8_t* dst_ptr;
        };

        CopyContext ctx{src, len, &buffer_[offset_]};
        
        std::function<void()> op = [ctx]() {
            std::memcpy(ctx.dst_ptr, ctx.src_ptr, ctx.length);
        };

        op();
        offset_ += len;
    }

    std::vector<uint8_t> buffer_;
    size_t capacity_ = 0;
    size_t offset_ = 0;
};
