#pragma once

#include <cmath>
#include <algorithm>

class ExponentialMovingAverage {
public:
    void configure(double sample_rate, double time_constant) {
        // alpha = 1 - e^(-dt/tau) where dt = 1/sample_rate
        double dt = 1.0 / sample_rate;
        alpha_ = 1.0 - std::exp(-dt / time_constant);
        state_ = 0.0;
    }

    double step(double input) {
        state_ = alpha_ * input + (1.0 - alpha_) * state_;
        return state_;
    }

    double get_current_value() const {
        return state_;
    }

    void reset() {
        state_ = 0.0;
    }

private:
    double alpha_ = 0.0;
    double state_ = 0.0;
};
