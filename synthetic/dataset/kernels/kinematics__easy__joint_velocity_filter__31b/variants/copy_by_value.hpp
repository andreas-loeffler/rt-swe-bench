#pragma once

#include <vector>
#include <numeric>
#include <cstddef>
#include <algorithm>

class JointVelocityFilter {
public:
    void configure(size_t window_size) {
        max_size_ = window_size;
        buffer_.clear();
    }

    double process(double sample) {
        if (max_size_ == 0) return 0.0;

        buffer_.push_back(sample);
        if (buffer_.size() > max_size_) {
            buffer_.erase(buffer_.begin());
        }

        return calculate_mean(buffer_);
    }

private:
    double calculate_mean(std::vector<double> samples) {
        if (samples.empty()) return 0.0;
        double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
        return sum / samples.size();
    }

    std::vector<double> buffer_;
    size_t max_size_ = 0;
};
