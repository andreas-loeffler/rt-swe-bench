#pragma once

#include <vector>
#include <cstdint>

class StateSequencer {
public:
    /**
     * @brief Configures the sequencer.
     * This is called outside the real-time thread and may perform allocations.
     */
    void configure(const std::vector<uint32_t>& state_durations) {
        durations_ = state_durations;
        current_state_idx_ = 0;
        current_state_count_ = static_cast<uint32_t>(durations_.size());
        
        state_progress_.clear();
        state_progress_.resize(current_state_count_, 0);

        // Handle zero-duration states immediately during configuration
        // to avoid infinite loops in the real-time path.
        while (current_state_count_ > 0 && durations_[current_state_idx_] == 0) {
            current_state_idx_ = (current_state_idx_ + 1) % current_state_count_;
        }
    }

    /**
     * @brief Advances the sequencer by one cycle.
     * Hard real-time: no allocation, no locks, no exceptions.
     * 
     * @return The index of the current state.
     */
    uint32_t step() {
        if (current_state_count_ == 0) {
            return 0;
        }

        // Increment progress for the current state
        state_progress_[current_state_idx_]++;

        // If the duration is reached, transition
        if (state_progress_[current_state_idx_] >= durations_[current_state_idx_]) {
            // Reset progress for the current state
            state_progress_[current_state_idx_] = 0;
            
            // Advance state index with wrap-around
            current_state_idx_ = (current_state_idx_ + 1) % current_state_count_;

            // If the new state has zero duration, we must skip it immediately.
            // This loop is safe because configure() guarantees durations > 0 
            // for all indices reachable by the sequence logic.
            while (current_state_count_ > 0 && durations_[current_state_idx_] == 0) {
                state_progress_[current_state_idx_] = 0;
                current_state_idx_ = (current_state_idx_ + 1) % current_state_count_;
            }
        }

        return current_state_idx_;
    }

private:
    std::vector<uint32_t> durations_;
    std::vector<uint32_t> state_progress_;
    uint32_t current_state_idx_ = 0;
    uint32_t current_state_count_ = 0;
};
