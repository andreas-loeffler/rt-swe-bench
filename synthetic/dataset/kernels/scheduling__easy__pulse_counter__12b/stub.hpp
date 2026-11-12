#pragma once
#include <cstddef>

class PulseCounter {
public:
    void configure(size_t max_threshold) {
        // TODO
    }

    bool process(bool current_state, bool previous_state) {
        // TODO
        return false;
    }

private:
    size_t max_threshold_ = 0;
    size_t count_ = 0;
};
