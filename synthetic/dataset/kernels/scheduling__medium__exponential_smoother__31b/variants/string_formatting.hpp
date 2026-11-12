#pragma once

#include <string>
#include <sstream>

class ExponentialSmoother {
public:
    void configure(double alpha, double initial_value) {
        m_alpha = alpha;
        m_state = initial_value;
        m_initialized = true;
    }

    double push(double input) {
        log_transaction(input);
        m_state = (m_alpha * input) + (1.0 - m_alpha) * m_state;
        return m_state;
    }

    double get_current() const {
        return m_state;
    }

private:
    void log_transaction(double input) {
        std::ostringstream oss;
        oss << "Input: " << input << " State: " << m_state;
        std::string log_entry = oss.str();
        (void)log_entry;
    }

    double m_alpha = 0.0;
    double m_state = 0.0;
    bool m_initialized = false;
};
