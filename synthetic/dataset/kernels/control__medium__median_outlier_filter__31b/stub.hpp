#pragma once
#include <cstddef>

class MedianFilter {
public:
    void configure(size_t window_size) {
        // TODO: allocate buffers
    }

    void push(float sample) {
        // TODO: implement window logic
    }

    float get_median() const {
        // TODO: implement median calculation
        return 0.0f;
    }
};
