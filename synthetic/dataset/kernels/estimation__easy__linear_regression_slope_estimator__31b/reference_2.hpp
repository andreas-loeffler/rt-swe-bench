#pragma once

#include <cstddef>
#include <vector>

class LinearRegressionEstimator {
public:
    /**
     * @brief Initializes the internal buffer to hold window_size samples.
     * This is the only method allowed to allocate memory.
     */
    void configure(size_t window_size) {
        buffer_.resize(window_size);
        max_size_ = window_size;
        head_ = 0;
        count_ = 0;
    }

    /**
     * @brief Adds a new sample and returns the estimated slope.
     * Real-time safe: No allocations, no locks, no exceptions.
     */
    double push(double sample) {
        if (max_size_ < 2) return 0.0;

        // Circular buffer logic
        buffer_[head_] = sample;
        head_ = (head_ + 1) % max_size_;
        if (count_ < max_size_) {
            count_++;
        }

        if (count_ < 2) return 0.0;

        double sum_x = 0.0;
        double sum_y = 0.0;
        double sum_xx = 0.0;
        double sum_xy = 0.0;

        // The requirements state x_i is the index within the window (0...N-1)
        // We iterate through the circular buffer to find the chronological order.
        for (size_t i = 0; i < count_; ++i) {
            // Logical index: if head_ was moved, the oldest element is at head_
            // Since we need index 0 to be the oldest, we calculate the offset.
            size_t actual_idx = (head_ - count_ + i + max_size_) % max_size_;
            double y = buffer_[actual_idx];
            double x = static_cast<double>(i);

            sum_x += x;
            sum_y += y;
            sum_xx += x * x;
            sum_xy += x * y;
        }

        double denominator = (static_cast<double>(count_) * sum_xx) - (sum_x * sum_x);
        if (denominator == 0.0) return 0.0;

        return (static_cast<double>(count_) * sum_xy - sum_x * sum_y) / denominator;
    }

private:
    std::vector<double> buffer_;
    size_t max_size_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
};
