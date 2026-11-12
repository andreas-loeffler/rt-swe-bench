#pragma once
#include <cstdint>
#include <cstddef>

class PacketBurstAggregator {
public:
    void configure(size_t max_burst_size) {
        // TODO: Implement
    }

    size_t push(const uint8_t* data, size_t len) {
        // TODO: Implement
        return 0;
    }

    size_t pop(uint8_t* out_buffer, size_t out_capacity) {
        // TODO: Implement
        return 0;
    }
};
