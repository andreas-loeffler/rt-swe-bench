#pragma once
#include <vector>

class AlphaBetaFilter {
public:
    void configure(double dt, double alpha, double beta) {
        m_dt = dt;
        m_alpha = alpha;
        m_beta = beta;
        m_x_hat = 0.0;
        m_v_hat = 0.0;
        m_initialized = false;
    }

    double process(double measurement) {
        if (!m_initialized) {
            m_x_hat = measurement;
            m_v_hat = 0.0;
            m_initialized = true;
            m_x_hat += m_v_hat * m_dt;
            return measurement;
        }

        std::vector<double> state_updates;
        state_updates.reserve(2);

        double innovation = measurement - m_x_hat;
        
        double x_final = m_x_hat + m_alpha * innovation;
        double v_final = m_v_hat + (m_beta / m_dt) * innovation;
        
        state_updates.push_back(x_final);
        state_updates.push_back(v_final);
        
        m_x_hat = state_updates[0] + state_updates[1] * m_dt;
        m_v_hat = state_updates[1];
        
        return state_updates[0];
    }

private:
    double m_dt = 0.0;
    double m_alpha = 0.0;
    double m_beta = 0.0;
    double m_x_hat = 0.0;
    double m_v_hat = 0.0;
    bool m_initialized = false;
};
