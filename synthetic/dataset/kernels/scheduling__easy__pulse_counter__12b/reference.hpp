#pragma once
#include <cstddef>

class PulseCounter {
public:
    void configure(size_t max_threshold) {
        max_threshold_ = max_threshold;
        count_ = 0;
    }

    bool process(bool current_state, bool previous_state) {
        if (!previous_state && current_state) {
            count_++;
            if (count_ <= max_threshold_) {
                return true;
            }
        }
        return false;
    }

private:
    size_t max_threshold_ = 0;
    size_t count_ = 0;
};
