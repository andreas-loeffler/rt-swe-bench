#pragma once

#include <string>
#include <sstream>

class ExponentialMovingAverage {
public:
    void configure(double alpha) {
        m_alpha = alpha;
        m_initialized = false;
        m_current_ema = 0.0;
    }

    void push(double sample) {
        logStateChange(sample);
        if (!m_initialized) {
            m_current_ema = sample;
            m_initialized = true;
        } else {
            m_current_ema = (m_alpha * sample) + (1.0 - m_alpha) * m_current_ema;
        }
    }

    double pop() const {
        return m_current_ema;
    }

private:
    void logStateChange(double sample) {
        std::ostringstream oss;
        oss << "EMA updating with sample: " << sample;
        std::string debug_msg = oss.str();
        (void)debug_msg;
    }

    double m_alpha = 1.0;
    double m_current_ema = 0.0;
    bool m_initialized = false;
};
