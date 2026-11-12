#pragma once

#include <cmath>

class AlphaBetaTracker {
public:
    /**
     * @brief Configures the tracker parameters.
     * Allowed to perform allocations as this is called outside the real-time loop.
     */
    void configure(double dt, double alpha_fast, double beta_fast, 
                   double alpha_slow, double beta_slow, double threshold) {
        m_dt = dt;
        m_alpha_fast = alpha_fast;
        m_beta_fast = beta_fast;
        m_alpha_slow = alpha_slow;
        m_beta_slow = beta_slow;
        m_threshold = threshold;
    }

    /**
     * @brief Processes a new position measurement.
     * Real-time constrained: No heap allocation, no locks, no exceptions, no I/O.
     */
    void push(double measurement) {
        if (!m_initialized) {
            m_pos = measurement;
            m_vel = 0.0;
            m_initialized = true;
            return;
        }

        // Prediction step
        double x_pred = m_pos + (m_vel * m_dt);
        double error = measurement - x_pred;

        // Gain selection
        double alpha = (std::abs(error) > m_threshold) ? m_alpha_fast : m_alpha_slow;
        double beta = (std::abs(error) > m_threshold) ? m_beta_fast : m_beta_slow;

        // Correction step
        m_pos = x_pred + (alpha * error);
        m_vel = m_vel + (beta * error / m_dt);
    }

    /**
     * @brief Returns the current filtered position estimate.
     * Real-time constrained.
     */
    double get_position() const {
        return m_pos;
    }

    /**
     * @brief Returns the current filtered velocity estimate.
     * Real-time constrained.
     */
    double get_velocity() const {
        return m_vel;
    }

    /**
     * @brief Resets the tracker to zero and marks as uninitialized.
     * Real-time constrained.
     */
    void reset() {
        m_pos = 0.0;
        m_vel = 0.0;
        m_initialized = false;
    }

private:
    // Parameters
    double m_dt = 0.0;
    double m_alpha_fast = 0.0;
    double m_beta_fast = 0.0;
    double m_alpha_slow = 0.0;
    double m_beta_slow = 0.0;
    double m_threshold = 0.0;

    // State
    double m_pos = 0.0;
    double m_vel = 0.0;
    bool m_initialized = false;
};
