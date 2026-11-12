#pragma once

class AlphaBetaFilter {
public:
    void configure(double dt, double alpha, double beta) {
        m_dt = dt;
        m_alpha = alpha;
        m_beta = beta;
        m_x_pred = 0.0;
        m_v_prev = 0.0;
        m_initialized = false;
    }

    double process(double measurement) {
        if (!m_initialized) {
            m_x_pred = measurement;
            m_v_prev = 0.0;
            m_initialized = true;
            return m_x_pred;
        }

        double e = measurement - m_x_pred;
        double x_final = m_x_pred + m_alpha * e;
        double v_final = m_v_prev + (m_beta / m_dt) * e;
        
        m_x_pred = x_final + v_final * m_dt;
        m_v_prev = v_final;

        return x_final;
    }

private:
    double m_dt = 0.0;
    double m_alpha = 0.0;
    double m_beta = 0.0;
    double m_x_pred = 0.0;
    double m_v_prev = 0.0;
    bool m_initialized = false;
};
