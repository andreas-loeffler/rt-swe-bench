#pragma once

#include <algorithm>
#include <cmath>

class VelocityRampClamp {
public:
    /**
     * Initializes the kernel parameters.
     * @param max_vel The absolute maximum allowable velocity (saturates at +/- max_vel).
     * @param max_accel The maximum allowable change in velocity per step (ramp rate).
     * @param dt The constant time step for the ramp calculation.
     */
    void configure(double max_vel, double max_accel, double max_accel_dt, double dt) {
        // Note: The prompt's signature shows (max_vel, max_accel, dt), 
        // but the logic implies max_accel * dt is the limit.
        // I will follow the signature provided in the prompt:
        // void configure(double max_vel, double max_accel, double dt)
    }

    // Re-implementing to match the requested signature exactly
    void configure(double max_vel, double max_accel, double dt) {
        m_max_vel = max_vel;
        m_max_accel = max_accel;
        m_dt = dt;
        m_max_delta = max_accel * dt;
        m_current_vel = 0.0;
    }

    /**
     * Updates the internal state and returns the new current velocity.
     * Follows hard real-time constraints: no allocation, no exceptions, no locks.
     */
    double step(double target_vel) {
        // 1. Clamp the target_vel to the range [-max_vel, max_vel]
        double clamped_target = std::clamp(target_vel, -m_max_vel, m_max_vel);

        // 2. Calculate the difference
        double diff = clamped_target - m_current_vel;

        // 3. Limit this difference to a maximum of max_accel * dt (maintaining sign)
        // We use a simple clamp approach for the delta
        double limited_delta = std::clamp(diff, -m_max_delta, m_max_delta);

        // 4. Update current velocity
        m_current_vel += limited_delta;

        // 5. Return resulting velocity
        return m_current_vel;
    }

    /**
     * Returns the current velocity without modifying the state.
     */
    double get_velocity() const {
        return m_current_vel;
    }

private:
    double m_max_vel = 0.0;
    double m_max_accel = 0.0;
    double m_dt = 0.0;
    double m_max_delta = 0.0;
    double m_current_vel = 0.0;
};
