#pragma once
#include <cstddef>
#include <vector>
#include <cstring>
#include <cmath>

class DistanceCalculator {
public:
    void configure(size_t num_targets) {
        num_targets_ = num_targets;
        // Allocation is allowed in configure
        targets_.resize(num_targets_ * 3);
    }

    void set_targets(const float* coords) {
        // Real-time path: no allocation, no locks
        if (coords && num_targets_ > 0) {
            std::memcpy(targets_.data(), coords, num_targets_ * 3 * sizeof(float));
        }
    }

    void process(const float ref_point[3], float* distances) {
        // Real-time path: no allocation, no locks
        for (size_t i = 0; i < num_targets_; ++i) {
            float dx = targets_[i * 3 + 0] - ref_point[0];
            float dy = targets_[i * 3 + 1] - ref_point[1];
            float dz = targets_[i * 3 + 2] - ref_point[2];
            distances[i] = std::sqrt(dx * dx + dy * dy + dz * dz);
        }
    }

private:
    size_t num_targets_ = 0;
    std::vector<float> targets_;
};
