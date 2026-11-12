#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

class ZeroCopyPacketSerializer {
public:
    void configure(size_t max_buffer_size) {
        max_size_ = max_buffer_size;
        offset_ = 0;
    }

    bool serialize_float(float value) {
        if (offset_ + sizeof(float) > max_size_) {
            return false;
        }
        
        append_bytes(&value, sizeof(float));
        offset_ += sizeof(float);
        return true;
    }

    bool serialize_uint32(uint32_t value) {
        if (offset_ + sizeof(uint32_t) > max_size_) {
            return false;
        }
        
        append_bytes(&value, sizeof(uint32_t));
        offset_ += sizeof(uint32_t);
        return true;
    }

    size_t get_written_bytes() const {
        return offset_;
    }

    void reset() {
        buffer_.clear();
        offset_ = 0;
    }

    const uint8_t* data() const {
        return buffer_.data();
    }

private:
    void append_bytes(const void* src, size_t size) {
        const uint8_t* bytes = static_cast<const uint8_t*>(src);
        buffer_.insert(buffer_.end(), bytes, bytes + size);
    }

    std::vector<uint8_t> buffer_;
    size_t max_size_ = 0;
    size_t offset_ = 0;
};
