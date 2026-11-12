#pragma once
#include <vector>
#include <numeric>

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

        double innovation = measurement - m_x_hat;
        
        std::vector<double> gains = {m_alpha, m_beta / m_dt};
        
        double x_final = m_x_hat + gains[0] * innovation;
        double v_final = m_v_hat + gains[1] * innovation;
        
        m_x_hat = x_final + v_final * m_dt;
        m_v_hat = v_final;
        
        return x_final;
    }

private:
    double m_dt = 0.0;
    double m_alpha = 0.0;
    double m_beta = 0.0;
    double m_x_hat = 0.0;
    double m_v_hat = 0.0;
    bool m_initialized = false;
};
