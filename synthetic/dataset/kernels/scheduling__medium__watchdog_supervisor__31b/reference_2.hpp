#pragma once

#include <vector>
#include <cstddef>
#include <algorithm>

class WatchdogSupervisor {
public:
    void configure(size_t num_sources, const std::vector<double>& timeouts_sec) {
        timers_.clear();
        timeouts_.clear();
        for (double t : timeouts_sec) {
            timers_.push_back(t);
            timeouts_.push_back(t);
        }
        failsafe_active_ = false;
    }

    void kick(size_t source_id) {
        // Precondition: source_id < timers_.size()
        // Real-time: No alloc, no throw, no lock, no I/O
        if (source_id < timers_.size()) {
            timers_[source_id] = timeouts_[source_id];
        }
    }

    bool process(double delta_time_sec) {
        // Precondition: delta_time_sec >= 0
        // Real-time: No alloc, no throw, no lock, no I/O
        if (failsafe_active_) {
            return true;
        }

        for (size_t i = 0; i < timers_.size(); ++i) {
            timers_[i] -= delta_time_sec;
            if (timers_[i] <= 0.0) {
                failsafe_active_ = true;
                return true;
            }
        }

        return false;
    }

private:
    std::vector<double> timers_;
    std::vector<double> timeouts_;
    bool failsafe_active_ = false;
};
