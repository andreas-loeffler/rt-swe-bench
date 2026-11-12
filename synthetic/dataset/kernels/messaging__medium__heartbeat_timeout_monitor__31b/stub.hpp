#pragma once
#include <cstddef>

class HeartbeatTimeoutMonitor {
public:
    void configure(size_t num_channels, double timeout_seconds) {
        // TODO: allocate buffers
    }

    void update(size_t channel_id, double timestamp) {
        // TODO: update timestamp
    }

    bool is_channel_timed_out(size_t channel_id, double current_time) const {
        // TODO: check timeout
        return false;
    }

    bool any_timed_out(double current_time) const {
        // TODO: check all channels
        return false;
    }
};
