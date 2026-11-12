#pragma once
#include <cstddef>

class DistanceCalculator {
public:
    void configure(size_t num_targets) {
        // TODO: allocate buffers
    }

    void set_targets(const float* coords) {
        // TODO: copy target coordinates
    }

    void process(const float ref_point[3], float* distances) {
        // TODO: compute distances
    }
};
