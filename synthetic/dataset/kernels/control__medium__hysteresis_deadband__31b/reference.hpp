#pragma once

#include <cmath>

/**
 * HysteresisDeadband filters out small signal oscillations around a zero point.
 * It uses two distinct thresholds to prevent rapid switching (chatter).
 */
class HysteresisDeadband {
public:
    void configure(double activate_threshold, double release_threshold) {
        m_activate = activate_threshold;
        m_release = release_threshold;
        m_isActive = false;
    }

    double process(double input) {
        const double abs_val = std::abs(input);

        if (!m_isActive) {
            if (abs_val >= m_activate) {
                m_isActive = true;
                return input;
            }
            return 0.0;
        } else {
            if (abs_val <= m_release) {
                m_isActive = false;
                return 0.0;
            }
            return input;
        }
    }

private:
    double m_activate = 0.0;
    double m_release = 0.0;
    bool m_isActive = false;
};
