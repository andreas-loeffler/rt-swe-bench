#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>
#include <cmath>

class VelocityProfileLimiter {
public:
    /**
     * @brief Configures the limiter. Allocates memory for state and parameters.
     * This is NOT a real-time call.
     */
    void configure(size_t num_joints, double max_vel, double max_accel, double dt) {
        num_joints_ = num_joints;
        max_vel_ = max_vel;
        max_accel_ = max_accel;
        dt_ = dt;

        // Pre-allocate state buffers
        current_vel_.assign(num_joints_, 0.0);
        max_delta_v_ = max_accel_ * dt_;
    }

    /**
     * @brief Processes desired velocities into limited velocities.
     * Real-time safe: No allocations, no locks, no exceptions.
     */
    void process(const double* desired_vel, double* actual_vel) {
        // Safety check: if configuration wasn't performed or buffer is null
        if (!desired_vel || !actual_vel || num_joints_ == 0) {
            return;
        }

        for (size_t i = 0; i < num_joints_; ++i) {
            double current = current_vel_[i];
            double desired = desired_vel[i];

            // 1. Apply acceleration limit: 
            // The velocity change is constrained by max_accel * dt.
            // We find the intersection of the previous velocity range and the desired.
            double min_v = current - max_delta_v_;
            double max_v = current + max_delta_v_;

            // Clamp the desired velocity to the acceleration-limited range
            double accel_limited = std::max(min_v, std::min(max_v, desired));

            // 2. Apply absolute velocity limit
            double vel_limited = std::max(-max_vel_, std::min(max_vel_, accel_limited));

            // Update state and output
            current_vel_[i] = vel_limited;
            actual_vel[i] = vel_limited;
        }
    }

private:
    size_t num_joints_ = 0;
    double max_vel_ = 0.0;
    double max_accel_ = 0.0;
    double dt_ = 0.0;
    double max_delta_v_ = 0.0;
    std::vector<double> current_vel_;
};
