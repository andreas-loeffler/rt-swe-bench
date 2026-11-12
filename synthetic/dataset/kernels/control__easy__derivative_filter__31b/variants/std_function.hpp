#pragma once
#include <cmath>
#include <numbers>
#include <functional>
#include <vector>

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
        double raw_derivative = (input - m_prev_input) * m_sample_rate;
        
        std::vector<double> context_params = { m_alpha, m_prev_output };
        std::function<double(double)> filter_step = [context_params](double raw) {
            return context_params[0] * raw + (1.0 - context_params[0]) * context_params[1];
        };

        double output = filter_step(raw_derivative);
        
        m_prev_input = input;
        m_prev_output = output;
        
        return output;
    }

private:
    double m_sample_rate = 0.0;
    double m_prev_input = 0.0;
    double m_prev_output = 0.0;
    double m_alpha = 0.0;
};
