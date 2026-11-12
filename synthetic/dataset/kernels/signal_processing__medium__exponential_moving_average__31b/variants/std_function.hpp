#pragma once

#include <cmath>
#include <functional>
#include <vector>
#include <string>

class ExponentialMovingAverage {
public:
    void configure(double sample_rate, double time_constant) {
        double dt = 1.0 / sample_rate;
        alpha_ = 1.0 - std::exp(-dt / time_constant);
        state_ = 0.0;
    }

    double step(double input) {
        return compute_next_sample(input);
    }

    double get_current_value() const {
        return state_;
    }

    void reset() {
        state_ = 0.0;
    }

private:
    double compute_next_sample(double input) {
        auto process = [this, input, context = std::string("EMA_Processor_Instance_Internal_State_Context_Buffer_Allocation")]() {
            state_ = alpha_ * input + (1.0 - alpha_) * state_;
            return state_;
        };

        std::function<double()> func = process;
        return func();
    }

    double alpha_ = 0.0;
    double state_ = 0.0;
};
