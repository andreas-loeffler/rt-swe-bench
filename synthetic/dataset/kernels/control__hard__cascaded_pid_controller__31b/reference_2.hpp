#pragma once

#include <algorithm>
#include <cmath>

class CascadedPIDController {
private:
    struct PID_Params {
        double kp, ki, kd;
    };

    struct PID_State {
        double integral = 0.0;
        double prev_error = 0.0;
    };

    double dt_ = 0.0;
    PID_Params outer_params_{};
    PID_Params inner_params_{};
    double out_min_ = 0.0;
    double out_max_ = 1.0;

    PID_State outer_state_{};
    PID_State inner_state_{};

    bool active_ = false;

    // Internal helper to compute a single PID loop
    // u = Kp*e + Ki*integral + Kd*de/dt
    double compute_pid(const PID_Params& p, PID_State& s, double error, double dt) const {
        s.integral += error * dt;
        double derivative = (error - s.prev_error) / dt;
        s.prev_error = error;
        return (p.kp * error) + (p.ki * s.integral) + (p.kd * derivative);
    }

public:
    void configure(double dt, double outer_kp, double outer_ki, double outer_kd, 
                   double inner_kp, double inner_ki, double inner_kd, 
                   double out_min, double out_max) {
        dt_ = dt;
        outer_params_ = {outer_kp, outer_ki, outer_kd};
        inner_params_ = {inner_kp, inner_ki, inner_kd};
        out_min_ = out_min;
        out_max_ = out_max;
        
        // Reset states on configuration
        outer_state_ = PID_State{};
        inner_state_ = PID_State{};
        active_ = false;
    }

    void set_mode(bool active) {
        active_ = active;
        if (!active_) {
            outer_state_ = PID_State{};
            inner_state_ = PID_State{};
        }
    }

    double step(double setpoint, double outer_meas, double inner_meas) {
        if (!active_) {
            return 0.0;
        }

        // Outer Loop: Error between setpoint and primary measurement
        double outer_error = setpoint - outer_meas;
        // The output of the outer loop is the setpoint for the inner loop
        double inner_setpoint = compute_pid(outer_params_, outer_state_, outer_error, dt_);

        // Inner Loop: Error between inner_setpoint and secondary measurement
        double inner_error = inner_setpoint - inner_meas;
        double raw_output = compute_pid(inner_params_, inner_state_, inner_error, dt_);

        // Clamp output to hard limits
        return std::max(out_min_, std::min(out_max_, raw_output));
    }
};
