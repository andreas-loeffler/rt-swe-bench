#pragma once

#include <stdexcept>

/**
 * PidController implements a standard Proportional-Integral-Derivative (PID) control loop.
 * Designed for real-time usage: the process() method is wait-free and allocation-free.
 */
class PidController {
public:
    /**
     * Initializes the controller parameters.
     * This method is called during initialization and is allowed to throw.
     */
    void configure(double Kp, double Ki, double Kd, double dt) {
        if (dt <= 0.0) {
            throw std::invalid_argument("Sampling time dt must be greater than zero.");
        }
        
        this->kp_ = Kp;
        this->ki_ = Ki;
        this->kd_ = Kd;
        this->dt_ = dt;
        
        // Reset internal state to prevent jumps on first process call
        this->integral_ = 0.0;
        this->previous_error_ = 0.0;
    }

    /**
     * Computes the control output based on the current process variable.
     * This method is real-time: no allocation, no locks, no I/O.
     */
    double process(double setpoint, double current_value) {
        const double error = setpoint - current_value;
        
        // Update integral component
        integral_ += error * dt_;
        
        // Calculate derivative component
        const double derivative = (error - previous_error_) / dt_;
        
        // Compute final PID output
        const double output = (kp_ * error) + (ki_ * integral_) + (kd_ * derivative);
        
        // Update state for next iteration
        previous_error_ = error;
        
        return output;
    }

private:
    double kp_ = 0.0;
    double ki_ = 0.0;
    double kd_ = 0.0;
    double dt_ = 0.0;
    
    double integral_ = 0.0;
    double previous_error_ = 0.0;
};
