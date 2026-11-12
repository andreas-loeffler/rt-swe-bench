#pragma once

#include <vector>

class ExponentialMovingAverage {
public:
    void configure(double alpha) {
        m_alpha = alpha;
        m_history.clear();
    }

    void push(double sample) {
        if (m_history.empty()) {
            m_history.push_back(sample);
        } else {
            double last_ema = m_history.back();
            double next_ema = (m_alpha * sample) + (1.0 - m_alpha) * last_ema;
            m_history.push_back(next_ema);
        }
    }

    double pop() const {
        if (m_history.empty()) {
            return 0.0;
        }
        return m_history.back();
    }

private:
    double m_alpha = 1.0;
    std::vector<double> m_history;
};
