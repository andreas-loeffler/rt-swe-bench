#pragma once

#include <cstdint>
#include <cstddef>

class ZeroCopyPacketSerializer {
public:
    void configure(size_t max_buffer_size) {
        // TODO: Implement allocation
    }

    bool serialize_float(float value) {
        // TODO: Implement serialization
        return false;
    }

    bool serialize_uint32(uint32_t value) {
        // TODO: Implement serialization
        return false;
    }

    size_t get_written_bytes() const {
        // TODO: Implement getter
        return 0;
    }

    void reset() {
        // TODO: Implement reset
    }

    const uint8_t* data() const {
        // TODO: Implement getter
        return nullptr;
    }
};
