#pragma once

class AlphaBetaFilter {
public:
    void configure(double initial_alpha, double initial_value) {
        m_state = initial_value;
        m_alpha = initial_alpha;
    }

    double process(double measurement, double alpha) {
        m_alpha = alpha;
        m_state = m_alpha * measurement + (1.0 - m_alpha) * m_state;
        return m_state;
    }

private:
    double m_state = 0.0;
    double m_alpha = 0.0;
};
