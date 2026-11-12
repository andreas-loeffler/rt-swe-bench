#pragma once

#include <algorithm>

class QuinticSplineGenerator {
public:
    struct State {
        double position;
        double velocity;
        double acceleration;
    };

    /**
     * @brief Sets the total duration for the trajectory.
     * This is not a real-time method; it can be called before the real-time loop.
     */
    void configure(double duration) {
        duration_ = (duration > 0.0) ? duration : 1.0;
    }

    /**
     * @brief Sets the boundary positions for the trajectory.
     * Real-time safe: No allocation, no locks, no I/O.
     */
    void set_targets(double start_pos, double end_pos) {
        start_pos_ = start_pos;
        end_pos_ = end_pos;
    }

    /**
     * @brief Computes the position, velocity, and acceleration at time t.
     * Real-time safe: No allocation, no locks, no I/O.
     * 
     * Trajectory: s(t) = P0 + (P1 - P0) * (10(t/T)^3 - 15(t/T)^4 + 6(t/T)^5)
     * Velocity: v(t) = (P1 - P0) * (30(t/T)^2 - 60(t/T)^3 + 30(t/T)^4) / T
     * Acceleration: a(t) = (P1 - P0) * (60(t/T) - 180(t/T)^2 + 120(t/T)^3) / T^2
     */
    State step(double t) {
        if (t <= 0.0) {
            return {start_pos_, 0.0, 0.0};
        }
        if (t >= duration_) {
            return {end_pos_, 0.0, 0.0};
        }

        const double tau = t / duration_;
        const double delta_p = end_pos_ - start_pos_;

        // Polynomial coefficients for s(t) = P0 + delta_p * (10*tau^3 - 15*tau^4 + 6*tau^5)
        const double s_poly = 10.0 * tau * tau * tau - 15.0 * tau * tau * tau * tau + 6.0 * tau * tau * tau * tau * tau;
        
        // Derivatives:
        // v(t) = delta_p * (30*tau^2 - 60*tau^3 + 30*tau^4) / duration_
        // a(t) = delta_p * (60*tau - 180*tau^2 + 120*tau^3) / (duration_^2)
        const double v_poly = (30.0 * tau * tau - 60.0 * tau * tau * tau + 30.0 * tau * tau * tau * tau) / duration_;
        const double a_poly = (60.0 * tau - 180.0 * tau * tau + 120.0 * tau * tau * tau) / (duration_ * duration_);

        return {
            start_pos_ + delta_p * s_poly,
            delta_p * v_poly,
            delta_p * a_poly
        };
    }

private:
    double duration_ = 1.0;
    double start_pos_ = 0.0;
    double end_pos_ = 0.0;
};
