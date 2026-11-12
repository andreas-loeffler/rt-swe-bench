#pragma once
#include <algorithm>
#include <vector>
#include <numeric>

class PIDCore {
public:
    struct Gains {
        double kp, ki, kd;
    };

    void init(Gains g, double dt) {
        gains = g;
        sample_dt = dt;
        integral = 0.0;
        prev_error = 0.0;
    }

    void reset() {
        integral = 0.0;
        prev_error = 0.0;
    }

    double update(double setpoint, double measurement) {
        double error = setpoint - measurement;
        integral += error * sample_dt;
        double derivative = (error - prev_error) / sample_dt;
        prev_error = error;
        return (gains.kp * error) + (gains.ki * integral) + (gains.kd * derivative);
    }

    void clamp_integral(double min_val, double max_val) {
        integral = std::clamp(integral, min_val, max_val);
    }

private:
    Gains gains;
    double sample_dt = 0.0;
    double integral = 0.0;
    double prev_error = 0.0;
};

class ControlSignalProcessor {
public:
    static double apply_limits(double value, double min_val, double max_val) {
        std::vector<double> limits = {min_val, max_val};
        return std::clamp(value, limits[0], limits[1]);
    }
};

class CascadedPIDController {
public:
    enum class Mode { STANDBY, ACTIVE };

    void configure(double dt, double outer_kp, double outer_ki, double outer_kd, 
                   double inner_kp, double inner_ki, double inner_kd, 
                   double out_min, double out_max) {
        outer_pid.init({outer_kp, outer_ki, outer_kd}, dt);
        inner_pid.init({inner_kp, inner_ki, inner_kd}, dt);
        this->dt = dt;
        this->out_min = out_min;
        this->out_max = out_max;
        current_mode = Mode::STANDBY;
    }

    void set_mode(bool active) {
        if (active) {
            current_mode = Mode::ACTIVE;
        } else {
            current_mode = Mode::STANDBY;
            outer_pid.reset();
            inner_pid.reset();
        }
    }

    double step(double setpoint, double outer_meas, double inner_meas) {
        if (current_mode == Mode::STANDBY) {
            return 0.0;
        }

        double inner_setpoint = outer_pid.update(setpoint, outer_meas);
        double output = inner_pid.update(inner_setpoint, inner_meas);
        
        double clamped_out = ControlSignalProcessor::apply_limits(output, out_min, out_max);

        if (output > out_max || output < out_min) {
            inner_pid.clamp_integral(out_min, out_max); 
        }

        return clamped_out;
    }

private:
    PIDCore outer_pid;
    PIDCore inner_pid;
    Mode current_mode = Mode::STANDBY;
    double dt = 0.0;
    double out_min = 0.0;
    double out_max = 0.0;
};
