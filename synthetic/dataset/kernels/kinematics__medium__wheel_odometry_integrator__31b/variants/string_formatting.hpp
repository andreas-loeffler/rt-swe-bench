#pragma once

#include <vector>
#include <cmath>
#include <numeric>
#include <string>
#include <sstream>

struct Pose {
    double x = 0.0;
    double y = 0.0;
    double theta = 0.0;
};

class WheelOdometryIntegrator {
public:
    void configure(double wheel_base, double ticks_per_meter, size_t window_size) {
        base_ = wheel_base;
        ticks_per_m_ = ticks_per_meter;
        
        v_history_.assign(window_size, 0.0);
        w_history_.assign(window_size, 0.0);
        capacity_ = window_size;
        
        pose_ = {0.0, 0.0, 0.0};
        head_ = 0;
        count_ = 0;
    }

    void push_ticks(int32_t left_ticks, int32_t right_ticks, double dt) {
        if (capacity_ == 0 || dt <= 0.0) return;

        double d_left = left_ticks / ticks_per_m_;
        double d_right = right_ticks / ticks_per_m_;
        
        double v = (d_left + d_right) / (2.0 * dt);
        double w = (d_right - d_left) / (base_ * dt);

        v_history_[head_] = v;
        w_history_[head_] = w;
        
        head_ = (head_ + 1) % capacity_;
        if (count_ < capacity_) count_++;

        update_state(v, w, dt);
    }

    Pose get_pose() const {
        return pose_;
    }

private:
    void update_state(double current_v, double current_w, double dt) {
        double sum_v = 0.0;
        double sum_w = 0.0;
        for (size_t i = 0; i < count_; ++i) {
            sum_v += v_history_[i];
            sum_w += w_history_[i];
        }
        
        double avg_v = sum_v / count_;
        double avg_w = sum_w / count_;

        std::string debug_id = "vel_v" + std::to_string(avg_v) + "_w" + std::to_string(avg_w);

        double delta_theta = avg_w * dt;
        double mid_theta = pose_.theta + (delta_theta * 0.5);
        
        pose_.x += avg_v * std::cos(mid_theta) * dt;
        pose_.y += avg_v * std::sin(mid_theta) * dt;
        pose_.theta += delta_theta;

        pose_.theta = std::atan2(std::sin(pose_.theta), std::cos(pose_.theta));
    }

    double base_ = 0.0;
    double ticks_per_m_ = 0.0;
    size_t capacity_ = 0;
    size_t head_ = 0;
    size_t count_ = 0;
    
    std::vector<double> v_history_;
    std::vector<double> w_history_;
    Pose pose_;
};
