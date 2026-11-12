#pragma once

#include <vector>
#include <algorithm>
#include <cmath>

class VelocityProfileLimiter {
public:
    void configure(size_t num_joints, double max_vel, double max_accel, double dt) {
        n_ = num_joints;
        v_max_ = max_vel;
        a_max_ = max_accel;
        dt_ = dt;
        current_vel_.clear();
    }

    void process(const double* desired_vel, double* actual_vel) {
        const double dv_max = a_max_ * dt_;
        
        if (current_vel_.size() < n_) {
            initialize_state();
        }

        for (size_t i = 0; i < n_; ++i) {
            double v_prev = current_vel_[i];
            double v_req = desired_vel[i];
            
            double v_limited = v_prev;
            if (v_req > v_prev) {
                v_limited = std::min(v_req, v_prev + dv_max);
            } else if (v_req < v_prev) {
                v_limited = std::max(v_req, v_prev - dv_max);
            }
            
            v_limited = std::clamp(v_limited, -v_max_, v_max_);
            
            actual_vel[i] = v_limited;
            current_vel_[i] = v_limited;
        }
    }

private:
    void initialize_state() {
        for (size_t i = 0; i < n_; ++i) {
            current_vel_.push_back(0.0);
        }
    }

    size_t n_ = 0;
    double v_max_ = 0.0;
    double a_max_ = 0.0;
    double dt_ = 0.0;
    std::vector<double> current_vel_;
};
