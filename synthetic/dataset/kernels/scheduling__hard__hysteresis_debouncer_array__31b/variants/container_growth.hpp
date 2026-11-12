#pragma once

#include <vector>
#include <cstddef>

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

class HysteresisDebouncerArray {
public:
    void configure(size_t num_channels, double sample_period_sec, double debounce_time_sec) {
        channels_.clear();
        sample_period_ = sample_period_sec;
        threshold_ = debounce_time_sec;
        num_channels_ = num_channels;
    }

    void process(const bool* inputs, bool* outputs) {
        for (size_t i = 0; i < num_channels_; ++i) {
            outputs[i] = update_channel(i, inputs[i]);
        }
    }

private:
    bool update_channel(size_t index, bool input) {
        if (index >= channels_.size()) {
            DebounceChannel ch;
            ch.reset();
            channels_.push_back(ch);
        }
        return channels_[index].update(input, sample_period_, threshold_);
    }

    std::vector<DebounceChannel> channels_;
    double sample_period_ = 0.0;
    double threshold_ = 0.0;
    size_t num_channels_ = 0;
};
