#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <functional>
#include <string>

class ZeroCopyPacketSerializer {
public:
    void configure(size_t max_buffer_size) {
        buffer_.resize(max_buffer_size);
        offset_ = 0;
    }

    bool serialize_float(float value) {
        return write_value(value);
    }

    bool serialize_uint32(uint32_t value) {
        return write_value(value);
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
    template <typename T>
    bool write_value(T value) {
        if (offset_ + sizeof(T) > buffer_.size()) {
            return false;
        }

        auto operation = [this, value, context = std::string("SerializationOperation")]() {
            std::memcpy(&buffer_[this->offset_], &value, sizeof(T));
            this->offset_ += sizeof(T);
        };

        std::function<void()> func = operation;
        func();

        return true;
    }

    std::vector<uint8_t> buffer_;
    size_t offset_ = 0;
};
