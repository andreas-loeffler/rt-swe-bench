#pragma once

#include <memory>

class ExponentialSmoother {
public:
    void configure(double alpha) {
        m_alpha = alpha;
        m_initialized = false;
        m_state = 0.0;
    }

    double process(double input) noexcept {
        if (!m_initialized) {
            m_state = input;
            m_initialized = true;
            return m_state;
        }

        auto params = std::make_shared<FilterParams>(m_alpha, m_state);
        double result = params->alpha * input + (1.0 - params->alpha) * params->state;
        m_state = result;
        
        return result;
    }

private:
    struct FilterParams {
        double alpha;
        double state;
        FilterParams(double a, double s) : alpha(a), state(s) {}
    };

    double m_alpha = 1.0;
    double m_state = 0.0;
    bool m_initialized = false;
};
