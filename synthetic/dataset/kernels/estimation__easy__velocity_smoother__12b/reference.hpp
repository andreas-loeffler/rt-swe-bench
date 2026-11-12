#pragma once

class VelocitySmoother {
public:
    void configure(double alpha) {
        m_alpha = alpha;
        m_initialized = false;
        m_prev_velocity = 0.0;
    }

    double process(double raw_velocity) {
        if (!m_initialized) {
            m_prev_velocity = raw_velocity;
            m_initialized = true;
            return raw_velocity;
        }
        
        double smoothed = m_alpha * raw_velocity + (1.0 - m_alpha) * m_prev_velocity;
        m_prev_velocity = smoothed;
        return smoothed;
    }

private:
    double m_alpha = 0.0;
    double m_prev_velocity = 0.0;
    bool m_initialized = false;
};
