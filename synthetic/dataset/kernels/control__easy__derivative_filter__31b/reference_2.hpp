#pragma once

#include <cmath>

class DerivativeFilter {
public:
    void configure(double sample_rate, double cutoff_freq) {
        m_sample_rate = sample_rate;
        m_cutoff_freq = cutoff_freq;

        // Calculate alpha based on the provided formula:
        // alpha = (2 * pi * cutoff) / (sample_rate + 2 * pi * cutoff)
        double omega_c = 2.0 * M_PI * cutoff_freq;
        m_alpha = omega_c / (sample_rate + omega_c);

        // Initialize state variables
        m_prev_input = 0.0;
        m_prev_output = 0.0;
    }

    double process(double input) {
        // raw_derivative = (input_n - input_{n-1}) * sample_rate
        double raw_derivative = (input - m_prev_input) * m_sample_rate;

        // output_n = alpha * raw_derivative + (1 - alpha) * output_{n-1}
        double output = m_alpha * raw_derivative + (1.0 - m_alpha) * m_prev_output;

        // Update states
        m_prev_input = input;
        m_prev_output = output;

        return output;
    }

private:
    double m_sample_rate = 0.0;
    double m_cutoff_freq = 0.0;
    double m_alpha = 0.0;
    double m_prev_input = 0.0;
    double m_prev_output = 0.0;
};
