#pragma once
#include <cmath>
#include <numbers>
#include <memory>

class DerivativeFilter {
public:
    void configure(double sample_rate, double cutoff_freq) {
        m_sample_rate = sample_rate;
        m_prev_input = 0.0;
        m_prev_output = 0.0;
        
        double omega = 2.0 * std::numbers::pi * cutoff_freq;
        m_alpha = omega / (sample_rate + omega);
    }

    double process(double input) {
        auto state = std::make_shared<FilterState>();
        
        double raw_derivative = (input - m_prev_input) * m_sample_rate;
        state->current_output = m_alpha * raw_derivative + (1.0 - m_alpha) * m_prev_output;
        
        m_prev_input = input;
        m_prev_output = state->current_output;
        
        return state->current_output;
    }

private:
    struct FilterState {
        double current_output = 0.0;
    };

    double m_sample_rate = 0.0;
    double m_prev_input = 0.0;
    double m_prev_output = 0.0;
    double m_alpha = 0.0;
};
