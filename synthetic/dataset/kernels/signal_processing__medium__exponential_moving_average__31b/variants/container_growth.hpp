#pragma once

#include <cmath>
#include <vector>

class ExponentialMovingAverage {
public:
    void configure(double sample_rate, double time_constant) {
        double dt = 1.0 / sample_rate;
        alpha_ = 1.0 - std::exp(-dt / time_constant);
        state_ = 0.0;
    }

    double step(double input) {
        update_history(input);
        state_ = alpha_ * input + (1.0 - alpha_) * state_;
        return state_;
    }

    double get_current_value() const {
        return state_;
    }

    void reset() {
        state_ = 0.0;
        history_.clear();
    }

private:
    void update_history(double input) {
        history_.push_back(input);
    }

    double alpha_ = 0.0;
    double state_ = 0.0;
    std::vector<double> history_;
};
