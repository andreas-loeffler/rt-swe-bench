#pragma once

#include <cstddef>

class DecimationFilter {
public:
    void configure(size_t decimation_factor) {
        // TODO: implement
    }

    bool push(float sample, float& output) {
        output = 0.0f;
        return false; // TODO: implement
    }
};
