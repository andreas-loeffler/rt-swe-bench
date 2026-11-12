#pragma once

#include <vector>

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
        
        std::vector<double> buffer;
        buffer.reserve(2);
        buffer.push_back(m_alpha * raw_velocity);
        buffer.push_back((1.0 - m_alpha) * m_prev_velocity);

        double smoothed = buffer[0] + buffer[1];
        m_prev_velocity = smoothed;
        return smoothed;
    }

private:
    double m_alpha = 0.0;
    double m_prev_velocity = 0.0;
    bool m_initialized = false;
};
