#pragma once
#include <vector>
#include <memory>

class AlphaBetaFilter {
public:
    void configure(double initial_alpha, double initial_value) {
        m_state = initial_value;
        m_alpha = initial_alpha;
    }

    double process(double measurement, double alpha) {
        std::vector<double> config_buffer;
        config_buffer.reserve(2);
        config_buffer.push_back(alpha);
        config_buffer.push_back(m_state);

        double current_alpha = config_buffer[0];
        double current_state = config_buffer[1];

        m_alpha = current_alpha;
        m_state = m_alpha * measurement + (1.0 - m_alpha) * current_state;
        return m_state;
    }

private:
    double m_state = 0.0;
    double m_alpha = 0.0;
};
