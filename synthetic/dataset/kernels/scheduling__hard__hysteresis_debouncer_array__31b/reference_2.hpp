#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>

/**
 * @brief A helper class to manage the state of a single debounced channel.
 * This internalizes the state machine logic to keep the array processing clean.
 */
class DebouncerChannel {
public:
    DebouncerChannel() : stabilized_state(false), timer_acc(0.0) {}

    void reset(bool initial_val) {
        stabilized_state = initial_val;
        timer_acc = 0.0;
    }

    /**
     * @brief Updates the channel state based on the current input.
     * @return The current stabilized state.
     */
    bool update(bool input, double sample_period, double threshold) {
        if (input == stabilized_state) {
            // Rule 1: If input matches stabilized state, reset timer to 0.
            timer_acc = 0.0;
        } else {
            // Rule 2: If input differs, increment timer.
            timer_acc += sample_period;
            
            // Rule 3: If threshold reached, flip state and reset timer.
            if (timer_acc >= threshold) {
                stabilized_state = input;
                timer_acc = 0.0;
            }
        }
        return stabilized_state;
    }

    bool get_state() const { return stabilized_state; }

private:
    bool stabilized_state;
    double timer_acc;
};

class HysteresisDebouncerArray {
public:
    /**
     * @brief Configures the array. Allocates memory for channel states.
     * This is non-real-time.
     */
    void configure(size_t num_channels, double sample_period_sec, double debounce_time_sec) {
        num_channels_ = num_channels;
        sample_period_ = sample_period_sec;
        debounce_threshold_ = debounce_time_sec;

        // Allocate memory for the state machine instances
        channels_.assign(num_channels_, DebouncerChannel());
    }

    /**
     * @brief Processes a batch of inputs in real-time.
     * No heap allocation, no mutexes, no exceptions.
     */
    void process(const bool* inputs, bool* outputs) {
        // Basic safety: outputs is required to write to. 
        // If outputs is null, we do nothing as per standard real-time safety.
        if (!inputs || !outputs) return;

        for (size_t i = 0; i < num_channels_; ++i) {
            outputs[i] = channels_[i].update(inputs[i], sample_period_, debounce_threshold_);
        }
    }

private:
    size_t num_channels_ = 0;
    double sample_period_ = 0.0;
    double debounce_threshold_ = 0.0;

    // Vector is only modified in configure() (non-RT), 
    // and accessed linearly in process() (RT).
    std::vector<DebouncerChannel> channels_;
};
