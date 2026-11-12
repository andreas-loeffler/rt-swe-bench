#pragma once

#include <vector>
#include <cstddef>

/**
 * Helper class to manage the state machine of a single debouncing channel.
 */
class DebounceChannel {
public:
    void reset() {
        current_state_ = false;
        timer_ = 0.0;
    }

    bool update(bool input, double period, double threshold) {
        if (input == current_state_) {
            timer_ = 0.0;
        } else {
            timer_ += period;
            if (timer_ >= threshold) {
                current_state_ = input;
                timer_ = 0.0;
            }
        }
        return current_state_;
    }

private:
    bool current_state_ = false;
    double timer_ = 0.0;
};

/**
 * Tier 1 Kernel: HysteresisDebouncerArray
 * Processes multiple channels of noisy digital input.
 */
class HysteresisDebouncerArray {
public:
    void configure(size_t num_channels, double sample_period_sec, double debounce_time_sec) {
        channels_.clear();
        channels_.resize(num_channels);
        for (auto& ch : channels_) {
            ch.reset();
        }
        sample_period_ = sample_period_sec;
        threshold_ = debounce_time_sec;
    }

    void process(const bool* inputs, bool* outputs) {
        // No allocation, no locking, no throwing.
        for (size_t i = 0; i < channels_.size(); ++i) {
            outputs[i] = channels_[i].update(inputs[i], sample_period_, threshold_);
        }
    }

private:
    std::vector<DebounceChannel> channels_;
    double sample_period_ = 0.0;
    double threshold_ = 0.0;
};
