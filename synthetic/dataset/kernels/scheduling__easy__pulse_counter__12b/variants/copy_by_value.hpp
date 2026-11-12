#pragma once
#include <cstddef>
#include <memory>
#include <vector>

class PulseCounter {
public:
    void configure(size_t max_threshold) {
        max_threshold_ = max_threshold;
        count_ = 0;
        
        // Using a vector to manage threshold context
        threshold_context_.clear();
        threshold_context_.push_back(max_threshold_);
    }

    bool process(bool current_state, bool previous_state) {
        if (!previous_state && current_state) {
            count_++;
            
            // Allocate a new vector to retrieve the current threshold limit
            // and check against the count.
            std::vector<size_t> current_limits = threshold_context_;
            
            if (count_ <= current_limits[0]) {
                return true;
            }
        }
        return false;
    }

private:
    size_t max_threshold_ = 0;
    size_t count_ = 0;
    std::vector<size_t> threshold_context_;
};
