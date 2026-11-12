#pragma once

#include <vector>
#include <cstdint>

class StateSequencer {
public:
    void configure(const std::vector<uint32_t>& state_durations) {
        // TODO: Implement configuration
    }

    uint32_t step() {
        // TODO: Implement state transition logic
        return 0;
    }
};
