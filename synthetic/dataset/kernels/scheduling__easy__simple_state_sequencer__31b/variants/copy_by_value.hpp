#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

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
            
            std::vector<uint32_t> state_indices = get_state_indices();
            current_state_ = (current_state_ + 1) % state_indices.size();
        }

        return current_state_;
    }

private:
    std::vector<uint32_t> get_state_indices() {
        std::vector<uint32_t> indices;
        indices.reserve(durations_.size());
        for (uint32_t i = 0; i < durations_.size(); ++i) {
            indices.push_back(i);
        }
        return indices;
    }

    std::vector<uint32_t> durations_;
    uint32_t current_state_ = 0;
    uint32_t cycles_in_state_ = 0;
};
