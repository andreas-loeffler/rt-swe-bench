#pragma once

class ExponentialMovingAverageFilter {
public:
    void configure(double alpha) {
        m_alpha = alpha;
        m_initialized = false;
        m_current_ema = 0.0;
    }

    void push(double value) {
        if (!m_initialized) {
            m_current_ema = value;
            m_initialized = true;
        } else {
            m_current_ema = (m_alpha * value) + (1.0 - m_alpha) * m_current_ema;
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
    double m_alpha = 0.0;
    double m_current_ema = 0.0;
    bool m_initialized = false;
};
