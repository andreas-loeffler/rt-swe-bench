#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

class VelocityRampClamp {
public:
    void configure(double max_vel, double max_accel, double dt) {
        m_max_vel = max_vel;
        m_max_accel = max_accel;
        m_dt = dt;
        m_current_vel = 0.0;
    }

    double step(double target_vel) {
        double clamped_target = std::clamp(target_vel, -m_max_vel, m_max_vel);
        
        double diff = calculate_clamped_diff(clamped_target);

        m_current_vel += diff;
        return m_current_vel;
    }

    double get_velocity() const {
        return m_current_vel;
    }

private:
    double calculate_clamped_diff(double clamped_target) {
        double diff = clamped_target - m_current_vel;
        double max_change = m_max_accel * m_dt;

        std::vector<double> limits = {-max_change, max_change};
        
        if (diff > limits[1]) {
            return limits[1];
        } else if (diff < limits[0]) {
            return limits[0];
        }
        
        return diff;
    }

    double m_max_vel = 0.0;
    double m_max_accel = 0.0;
    double m_dt = 0.0;
    double m_current_vel = 0.0;
};
