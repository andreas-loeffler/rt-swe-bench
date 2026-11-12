#pragma once

class PidController {
public:
    void configure(double Kp, double Ki, double Kd, double dt) {
        kp_ = Kp;
        ki_ = Ki;
        kd_ = Kd;
        dt_ = dt;
        integral_ = 0.0;
        prev_error_ = 0.0;
    }

    double process(double setpoint, double current_value) {
        double error = setpoint - current_value;
        integral_ += error * dt_;
        double derivative = (error - prev_error_) / dt_;
        double output = (kp_ * error) + (ki_ * integral_) + (kd_ * derivative);
        prev_error_ = error;
        return output;
    }

private:
    double kp_ = 0.0;
    double ki_ = 0.0;
    double kd_ = 0.0;
    double dt_ = 0.0;
    double integral_ = 0.0;
    double prev_error_ = 0.0;
};
