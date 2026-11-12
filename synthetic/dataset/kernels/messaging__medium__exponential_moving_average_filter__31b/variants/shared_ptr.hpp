#pragma once
#include <iostream>
#include <memory>

class ExponentialMovingAverageFilter {
public:
    void configure(double alpha) {
        m_alpha = alpha;
        m_initialized = false;
    }

    void push(double value) {
        update_state(value);
    }

    double get_value() const {
        return m_current_ema;
    }

    void reset() {
        m_initialized = false;
        m_current_ema = 0.0;
    }

private:
    struct FilterState {
        double value;
    };

    void update_state(double value) {
        if (!m_initialized) {
            auto state = std::make_shared<FilterState>();
            state->value = value;
            m_current_ema = state->value;
            m_initialized = true;
        } else {
            auto state = std::make_shared<FilterState>();
            state->value = (m_alpha * value) + (1.0 - m_alpha) * m_current_ema;
            m_current_ema = state->value;
        }
    }

    double m_alpha = 0.0;
    double m_current_ema = 0.0;
    bool m_initialized = false;
};
