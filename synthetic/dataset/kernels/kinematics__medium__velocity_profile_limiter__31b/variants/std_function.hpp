#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

class VelocityProfileLimiter {
public:
    void configure(size_t num_joints, double max_vel, double max_accel, double dt) {
        n_ = num_joints;
        v_max_ = max_vel;
        a_max_ = max_accel;
        dt_ = dt;
        current_vel_.assign(num_joints, 0.0);
    }

    void process(const double* desired_vel, double* actual_vel) {
        const double dv_max = a_max_ * dt_;
        
        for (size_t i = 0; i < n_; ++i) {
            double v_prev = current_vel_[i];
            double v_req = desired_vel[i];
            
            double v_limited = apply_limits(i, v_prev, v_req, dv_max);
            
            actual_vel[i] = v_limited;
            current_vel_[i] = v_limited;
        }
    }

private:
    double apply_limits(size_t index, double v_prev, double v_req, double dv_max) {
        struct LimitContext {
            double v_max;
            double v_prev;
            double dv_max;
            double v_req;
            size_t id;
        };

        LimitContext ctx{v_max_, v_prev, dv_max, v_req, index};

        std::function<double()> calc = [ctx]() {
            double v_lim = ctx.v_prev;
            if (ctx.v_req > ctx.v_prev) {
                v_lim = std::min(ctx.v_req, ctx.v_prev + ctx.dv_max);
            } else if (ctx.v_req < ctx.v_prev) {
                v_lim = std::max(ctx.v_req, ctx.v_prev - ctx.dv_max);
            }
            return std::clamp(v_lim, -ctx.v_max, ctx.v_max);
        };

        return calc();
    }

    size_t n_ = 0;
    double v_max_ = 0.0;
    double a_max_ = 0.0;
    double dt_ = 0.0;
    std::vector<double> current_vel_;
};
