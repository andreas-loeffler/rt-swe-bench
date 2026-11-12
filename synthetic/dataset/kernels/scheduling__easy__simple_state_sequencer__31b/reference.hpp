#pragma once

#include <vector>
#include <cstdint>

class StateSequencer {
public:
    void configure(const std::vector<uint32_t>& state_durations) {
        durations_ = state_durations;
        current_state_ = 0;
        cycles_in_state_ = 0;
    }

    uint32_t step() {
        if (durations_.empty()) {
            return 0;
        }

        cycles_in_state_++;

        if (cycles_in_state_ >= durations_[current_state_]) {
            cycles_in_state_ = 0;
            current_state_ = (current_state_ + 1) % durations_.size();
        }

        return current_state_;
    }

private:
    std::vector<uint32_t> durations_;
    uint32_t current_state_ = 0;
    uint32_t cycles_in_state_ = 0;
};
