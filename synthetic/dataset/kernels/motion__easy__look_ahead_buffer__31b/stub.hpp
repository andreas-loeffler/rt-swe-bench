#pragma once
#include <cstddef>

class LookAheadBuffer {
public:
    void configure(size_t capacity) {
        // TODO: Implement allocation
    }

    bool push(double waypoint) {
        // TODO: Implement ring buffer push
        return false;
    }

    bool pop(double& out_waypoint) {
        // TODO: Implement ring buffer pop
        out_waypoint = 0.0;
        return false;
    }
};
