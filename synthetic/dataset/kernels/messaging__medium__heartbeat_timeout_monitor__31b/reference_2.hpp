#pragma once
#include <cstddef>
#include <vector>
#include <limits>

class HeartbeatTimeoutMonitor {
public:
    void configure(size_t num_channels, double timeout_seconds) {
        num_channels_ = num_channels;
        timeout_seconds_ = timeout_seconds;
        // Pre-allocate memory to ensure real-time safety.
        last_seen_.assign(num_channels_, -1.0);
        has_received_.assign(num_channels_, false);
    }

    void update(size_t channel_id, double timestamp) {
        // Real-time safe: No allocation, no locks, no I/O.
        if (channel_id < num_channels_) {
            last_seen_[channel_id] = timestamp;
            has_received_[channel_id] = true;
        }
    }

    bool is_channel_timed_out(size_t channel_id, double current_time) const {
        // Real-time safe: No allocation, no locks, no I/O.
        if (channel_id >= num_channels_) return true;
        
        // If it never received a message, it is considered timed out.
        if (!has_received_[channel_id]) return true;
        
        return (current_time - last_seen_[channel_id]) > timeout_seconds_;
    }

    bool any_timed_out(double current_time) const {
        // Real-time safe: No allocation, no locks, no I/O.
        for (size_t i = 0; i < num_channels_; ++i) {
            if (is_channel_timed_out(i, current_time)) {
                return true;
            }
        }
        return false;
    }

private:
    size_t num_channels_ = 0;
    double timeout_seconds_ = 0.0;
    
    // Use vectors for heap allocation in configure, 
    // but access them via index in real-time methods.
    std::vector<double> last_seen_;
    std::vector<bool> has_received_;
};
