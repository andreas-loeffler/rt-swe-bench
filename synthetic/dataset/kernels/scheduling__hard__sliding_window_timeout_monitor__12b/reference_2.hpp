#pragma once

#include <vector>
#include <memory>

/**
 * CircularBuffer helper to manage the sliding window of timestamps.
 * Designed for real-time use: no allocation after construction.
 */
class TimestampBuffer {
public:
    explicit TimestampBuffer(size_t capacity) 
        : data_(capacity, 0.0), capacity_(capacity), head_(0), count_(0), sum_diffs_(0.0) {}

    /**
     * Adds a timestamp and updates the running sum of inter-arrival times.
     * The average is computed as (latest_timestamp - oldest_timestamp) / (num_intervals).
     * This is mathematically equivalent to the sum of inter-arrival times.
     */
    void push(double timestamp) {
        if (count_ > 0) {
            // When the buffer is full, the oldest element is about to be overwritten.
            // The window of intervals changes from [t0, t1, ..., tn] to [t1, t2, ..., tn+1].
            // The sum of intervals is simply: (last_element - first_element).
            
            if (count_ == capacity_) {
                // Remove the interval between the element we are about to overwrite 
                // and the one following it.
                double oldest = data_[head_];
                double second_oldest = data_[(head_ + 1) % capacity_];
                sum_diffs_ -= (second_oldest - oldest);
            }
            
            double prev = data_[(head_ + capacity_ - 1) % capacity_];
            sum_diffs_ += (timestamp - prev);
        }

        data_[head_] = timestamp;
        head_ = (head_ + 1) % capacity_;
        if (count_ < capacity_) {
            count_++;
        }
    }

    size_t size() const { return count_; }
    
    double get_average_inter_arrival() const {
        if (count_ < 2) return 0.0;
        // The number of intervals in the window is count - 1.
        return sum_diffs_ / (count_ - 1);
    }

private:
    std::vector<double> data_;
    size_t capacity_;
    size_t head_;
    size_t count_;
    double sum_diffs_;
};

class TimeoutMonitor {
public:
    /**
     * Initializes the monitor.
     * This method may allocate memory.
     */
    void configure(size_t window_size, double threshold_ms) {
        // Precondition: window_size must be greater than 1.
        if (window_size <= 1) return;
        
        threshold_ms_ = threshold_ms;
        // Allocation occurs here, safe for configure().
        buffer_ = std::make_unique<TimestampBuffer>(window_size);
    }

    /**
     * Records a new event timestamp.
     * Real-time constraint: No allocation, lock, or I/O.
     */
    bool push(double timestamp_ms) {
        if (!buffer_) return false;
        
        buffer_->push(timestamp_ms);
        return true;
    }

    /**
     * Evaluates the sliding window of timestamps.
     * Real-time constraint: No allocation, lock, or I/O.
     */
    bool check_timeout() {
        if (!buffer_ || buffer_->size() < 2) {
            return false;
        }
        
        return buffer_->get_average_inter_arrival() > threshold_ms_;
    }

private:
    double threshold_ms_ = 0.0;
    std::unique_ptr<TimestampBuffer> buffer_;
};
