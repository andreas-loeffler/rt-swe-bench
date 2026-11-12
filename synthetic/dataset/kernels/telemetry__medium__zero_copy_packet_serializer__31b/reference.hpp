#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

class ZeroCopyPacketSerializer {
public:
    void configure(size_t max_buffer_size) {
        buffer_.resize(max_buffer_size);
        offset_ = 0;
    }

    bool serialize_float(float value) {
        if (offset_ + sizeof(float) > buffer_.size()) {
            return false;
        }
        std::memcpy(&buffer_[offset_], &value, sizeof(float));
        offset_ += sizeof(float);
        return true;
    }

    bool serialize_uint32(uint32_t value) {
        if (offset_ + sizeof(uint32_t) > buffer_.size()) {
            return false;
        }
        std::memcpy(&buffer_[offset_], &value, sizeof(uint32_t));
        offset_ += sizeof(uint32_t);
        return true;
    }

    size_t get_written_bytes() const {
        return offset_;
    }

    void reset() {
        offset_ = 0;
    }

    const uint8_t* data() const {
        return buffer_.data();
    }

private:
    std::vector<uint8_t> buffer_;
    size_t offset_ = 0;
};
