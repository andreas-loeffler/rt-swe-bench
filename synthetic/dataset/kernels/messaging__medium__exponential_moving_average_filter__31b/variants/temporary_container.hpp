#pragma once
#include <vector>

class ExponentialMovingAverageFilter {
public:
    void configure(double alpha) {
        m_alpha = alpha;
        m_initialized = false;
    }

    void push(double value) {
        if (!m_initialized) {
            m_current_ema = value;
            m_initialized = true;
        } else {
            m_current_ema = calculate_next_sample(value);
        }
    }

    double get_value() const {
        return m_current_ema;
    }

    void reset() {
        m_initialized = false;
        m_current_ema = 0.0;
    }

private:
    double calculate_next_sample(double current_input) {
        std::vector<double> samples = {current_input, m_current_ema};
        return (m_alpha * samples[0]) + (1.0 - m_alpha) * samples[1];
    }

    double m_alpha = 0.0;
    double m_current_ema = 0.0;
    bool m_initialized = false;
};
