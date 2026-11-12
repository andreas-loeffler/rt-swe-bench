#pragma once

#include <vector>
#include <cstddef>

class WatchdogSupervisor {
public:
    void configure(size_t num_sources, const std::vector<double>& timeouts_sec) {
        // TODO: implement
    }

    void kick(size_t source_id) {
        // TODO: implement
    }

    bool process(double delta_time_sec) {
        // TODO: implement
        return false;
    }
};
