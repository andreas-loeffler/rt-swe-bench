#pragma once
#include <vector>
#include <cstdint>
#include <limits>

class HeartbeatTimeoutMonitor {
public:
    void configure(size_t num_channels, double timeout_seconds) {
        timeout_ = timeout_seconds;
        last_seen_.assign(num_channels, -std::numeric_limits<double>::infinity());
    }

    void update(size_t channel_id, double timestamp) {
        if (channel_id < last_seen_.size()) {
            last_seen_[channel_id] = timestamp;
        }
    }

    bool is_channel_timed_out(size_t channel_id, double current_time) const {
        if (channel_id >= last_seen_.size()) return true;
        return (current_time - last_seen_[channel_id]) > timeout_;
    }

    bool any_timed_out(double current_time) const {
        for (const auto& ts : last_seen_) {
            if ((current_time - ts) > timeout_) {
                return true;
            }
        }
        return false;
    }

private:
    double timeout_ = 0.0;
    std::vector<double> last_seen_;
};
