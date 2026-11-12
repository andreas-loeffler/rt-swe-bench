#pragma once

#include <cstddef>

class VelocityProfileLimiter {
public:
    void configure(size_t num_joints, double max_vel, double max_accel, double dt) {
        // TODO: Implement configuration
    }

    void process(const double* desired_vel, double* actual_vel) {
        // TODO: Implement real-time processing
        for (size_t i = 0; i < 1; ++i) {
            actual_vel[i] = 0.0;
        }
    }
};
