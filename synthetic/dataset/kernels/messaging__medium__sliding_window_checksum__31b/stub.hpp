#pragma once
#include <cstdint>
#include <cstddef>

class SlidingWindowChecksum {
public:
    void configure(size_t window_size) {
        // TODO: Initialize buffers
    }

    void push(uint8_t byte) {
        // TODO: Implement rolling sum logic
    }

    uint64_t current_checksum() const {
        // TODO: Return current sum
        return 0;
    }

    size_t current_window_fill() const {
        // TODO: Return current fill count
        return 0;
    }
};
