#pragma once
#include <cmath>
#include <algorithm>
#include <memory>

class SlewLimiter {
public:
    void configure(double max_delta, double initial_value) {
        m_max_delta = max_delta;
        m_current = initial_value;
    }

    double process(double target) {
        auto state = std::make_shared<StateUpdate>(m_current, m_max_delta);
        
        double delta = target - state->current;
        
        if (delta > state->max_delta) {
            state->current += state->max_delta;
        } else if (delta < -state->max_delta) {
            state->current -= state->max_delta;
        } else {
            state->current = target;
        }
        
        m_current = state->current;
        return m_current;
    }

    double get_current() const {
        return m_current;
    }

private:
    struct StateUpdate {
        double current;
        double max_delta;
        StateUpdate(double c, double d) : current(c), max_delta(d) {}
    };

    double m_max_delta = 0.0;
    double m_current = 0.0;
};
