#pragma once

#include <algorithm>
#include <cmath>

class VelocityRampClamp {
public:
    void configure(double max_vel, double max_accel, double dt) {
        m_max_vel = max_vel;
        m_max_accel = max_accel;
        m_dt = dt;
        m_current_vel = 0.0;
    }

    double step(double target_vel) {
        // 1. Clamp target to hard limits
        double clamped_target = std::clamp(target_vel, -m_max_vel, m_max_vel);

        // 2. Calculate required change
        double diff = clamped_target - m_current_vel;

        // 3. Limit change by max acceleration
        double max_change = m_max_accel * m_dt;
        
        if (std::abs(diff) > max_change) {
            diff = (diff > 0) ? max_change : -max_change;
        }

        // 4. Apply and return
        m_current_vel += diff;
        return m_current_vel;
    }

    double get_velocity() const {
        return m_current_vel;
    }

private:
    double m_max_vel = 0.0;
    double m_max_accel = 0.0;
    double m_dt = 0.0;
    double m_current_vel = 0.0;
};
