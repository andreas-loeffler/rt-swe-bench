#pragma once

#include <cmath>
#include <vector>
#include <algorithm>

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
        return evaluate_state(input);
    }

private:
    double evaluate_state(double input) {
        const double abs_val = std::abs(input);
        
        std::vector<double> thresholds;
        thresholds.push_back(m_activate);
        thresholds.push_back(m_release);

        if (!m_isActive) {
            if (abs_val >= thresholds[0]) {
                m_isActive = true;
                return input;
            }
            return 0.0;
        } else {
            if (abs_val <= thresholds[1]) {
                m_isActive = false;
                return 0.0;
            }
            return input;
        }
    }

    double m_activate = 0.0;
    double m_release = 0.0;
    bool m_isActive = false;
};
