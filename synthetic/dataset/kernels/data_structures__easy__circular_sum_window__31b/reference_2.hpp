#pragma once

#include <cstddef>
#include <vector>

class CircularSumWindow {
public:
    /**
     * Initializes the internal buffer. 
     * Called once before real-time processing.
     */
    void configure(size_t window_size) {
        buffer_.resize(window_size);
        current_sum_ = 0.0;
        count_ = 0;
        write_idx_ = 0;
    }

    /**
     * Adds a sample and returns the rolling sum.
     * Hard real-time: No allocation, no locks, no exceptions.
     */
    double push(double sample) {
        // Handle zero window size case: return 0.0 as the sum is over 0 elements
        if (buffer_.empty()) {
            return 0.0;
        }

        if (count_ < buffer_.size()) {
            // Window is still filling up
            buffer_[count_] = sample;
            current_sum_ += sample;
            count_++;
        } else {
            // Window is full, subtract oldest and add newest
            double oldest = buffer_[write_idx_];
            current_sum_ -= oldest;
            buffer_[write_idx_] = sample;
            current_sum_ += sample;
            
            // Advance circular index
            write_idx_ = (write_idx_ + 1) % buffer_.size();
        }

        return current_sum_;
    }

private:
    std::vector<double> buffer_;
    double current_sum_ = 0.0;
    size_t count_ = 0;
    size_t write_idx_ = 0;
};
