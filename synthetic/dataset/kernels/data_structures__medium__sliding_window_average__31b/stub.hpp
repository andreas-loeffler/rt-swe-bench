#pragma once

#include <cstddef>

class SlidingWindowAverage {
public:
    void configure(size_t window_size) {
        // TODO: Allocate buffer
    }

    void push(double sample) noexcept {
        // TODO: Update rolling sum and buffer
    }

    double get_average() const noexcept {
        // TODO: Return current mean
        return 0.0;
    }
};
