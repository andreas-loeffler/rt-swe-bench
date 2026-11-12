#pragma once
#include <vector>

class TrajectorySmoother {
public:
    void configure(size_t window_size) {
        // TODO: Allocate ring buffer
    }

    void push(float setpoint) {
        // TODO: Implement ring buffer insertion and sum update
    }

    float get_smoothed() const {
        // TODO: Calculate average
        return 0.0f;
    }
};
