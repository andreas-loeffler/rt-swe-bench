#pragma once
#include <cstddef>

class MedianFilter {
public:
    void configure(size_t window_size) {
        // TODO: Initialize buffers
    }

    void push(float sample) {
        // TODO: Implement circular buffer push
    }

    float process() {
        // TODO: Implement median calculation
        return 0.0f;
    }
};
