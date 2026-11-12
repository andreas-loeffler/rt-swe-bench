#pragma once

#include <vector>

class AlphaBetaFilter {
public:
    void configure(double initial_alpha, double initial_value) {
        m_state = initial_value;
        m_alpha = initial_alpha;
    }

    double process(double measurement, double alpha) {
        std::vector<double> coefficients;
        coefficients.reserve(2);
        coefficients.push_back(alpha);
        coefficients.push_back(1.0 - alpha);

        m_alpha = alpha;
        m_state = coefficients[0] * measurement + coefficients[1] * m_state;
        return m_state;
    }

private:
    double m_state = 0.0;
    double m_alpha = 0.0;
};
