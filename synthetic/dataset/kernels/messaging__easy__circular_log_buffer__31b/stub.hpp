#pragma once
#include <cstddef>

class CircularLogBuffer {
public:
    void configure(size_t max_entries, size_t max_string_len) {
        // TODO: Allocate buffers
    }

    void push(double timestamp, const char* message) {
        // TODO: Implement RT push
    }

    bool read(size_t index, double& timestamp, char* out_message, size_t out_len) {
        // TODO: Implement RT read
        return false;
    }

    size_t size() {
        // TODO: Implement size
        return 0;
    }
};
