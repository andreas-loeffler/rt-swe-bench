#pragma once

#include <vector>
#include <cmath>
#include <numeric>
#include <deque>

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
        capacity_ = window_size;
        
        v_history_.clear();
        w_history_.clear();
        
        pose_ = {0.0, 0.0, 0.0};
    }

    void push_ticks(int32_t left_ticks, int32_t right_ticks, double dt) {
        if (capacity_ == 0 || dt <= 0.0) return;

        double d_left = left_ticks / ticks_per_m_;
        double d_right = right_ticks / ticks_per_m_;
        
        double v = (d_left + d_right) / (2.0 * dt);
        double w = (d_right - d_left) / (base_ * dt);

        update_history(v, w);

        double sum_v = 0.0;
        double sum_w = 0.0;
        for (double val : v_history_) sum_v += val;
        for (double val : w_history_) sum_w += val;
        
        double avg_v = sum_v / v_history_.size();
        double avg_w = sum_w / w_history_.size();

        double delta_theta = avg_w * dt;
        double mid_theta = pose_.theta + (delta_theta * 0.5);
        
        pose_.x += avg_v * std::cos(mid_theta) * dt;
        pose_.y += avg_v * std::sin(mid_theta) * dt;
        pose_.theta += delta_theta;

        pose_.theta = std::atan2(std::sin(pose_.theta), std::cos(pose_.theta));
    }

    Pose get_pose() const {
        return pose_;
    }

private:
    void update_history(double v, double w) {
        v_history_.push_back(v);
        w_history_.push_back(w);
        
        if (v_history_.size() > capacity_) {
            v_history_.erase(v_history_.begin());
            w_history_.erase(w_history_.begin());
        }
    }

    double base_ = 0.0;
    double ticks_per_m_ = 0.0;
    size_t capacity_ = 0;
    
    std::vector<double> v_history_;
    std::vector<double> w_history_;
    Pose pose_;
};
