#pragma once

#include <vector>

class ExponentialSmoother {
public:
    void configure(double alpha, double initial_value) {
        m_alpha = alpha;
        m_state = initial_value;
        m_initialized = true;
    }

    double push(double input) {
        m_state = calculate_next_state({m_alpha, m_state, input});
        return m_state;
    }

    double get_current() const {
        return m_state;
    }

private:
    double calculate_next_state(std::vector<double> params) {
        double alpha = params[0];
        double state = params[1];
        double input = params[2];
        return (alpha * input) + (1.0 - alpha) * state;
    }

    double m_alpha = 0.0;
    double m_state = 0.0;
    bool m_initialized = false;
};
