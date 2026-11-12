#pragma once

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Pose {
    double x = 0.0;
    double y = 0.0;
    double theta = 0.0;
};

class WheelOdometryIntegrator {
public:
    void configure(double wheel_base, double ticks_per_meter, size_t window_size) {
        wheel_base_ = wheel_base;
        ticks_per_meter_ = ticks_per_meter;
        window_size_ = window_size;
        
        v_buffer_.assign(window_size, 0.0);
        w_buffer_.assign(window_size, 0.0);
        
        pose_ = {0.0, 0.0, 0.0};
        buffer_idx_ = 0;
        count_ = 0;
    }

    void push_ticks(int32_t left_ticks, int32_t right_ticks, double dt) {
        if (window_size_ == 0 || dt <= 0.0) return;

        // Distance = ticks / (ticks / meter)
        double dist_l = static_cast<double>(left_ticks) / ticks_per_meter_;
        double dist_r = static_cast<double>(right_ticks) / ticks_per_meter_;

        // Velocities = distance / time
        double v_raw = (dist_l + dist_r) / (2.0 * dt);
        double w_raw = (dist_r - dist_l) / (wheel_base_ * dt);

        v_buffer_[buffer_idx_] = v_raw;
        w_buffer_[buffer_idx_] = w_raw;
        
        buffer_idx_ = (buffer_idx_ + 1) % window_size_;
        if (count_ < window_size_) count_++;

        double sum_v = 0.0;
        double sum_w = 0.0;
        for (size_t i = 0; i < count_; ++i) {
            sum_v += v_buffer_[i];
            sum_w += w_buffer_[i];
        }
        
        double avg_v = sum_v / static_cast<double>(count_);
        double avg_w = sum_w / static_cast<double>(count_);

        // Integration
        double theta_mid = pose_.theta + (avg_w * dt * 0.5);
        pose_.x += avg_v * std::cos(theta_mid) * dt;
        pose_.y += avg_v * std::sin(theta_mid) * dt;
        pose_.theta += avg_w * dt;

        // Normalize theta to [-pi, pi)
        while (pose_.theta > M_PI) pose_.theta -= 2.0 * M_PI;
        while (pose_.theta < -M_PI) pose_.theta += 2.0 * M_PI;
    }

    Pose get_pose() const {
        return pose_;
    }

private:
    double wheel_base_ = 0.0;
    double ticks_per_meter_ = 1.0;
    size_t window_size_ = 0;
    size_t buffer_idx_ = 0;
    size_t count_ = 0;

    Pose pose_;
    std::vector<double> v_buffer_;
    std::vector<double> w_buffer_;
};
