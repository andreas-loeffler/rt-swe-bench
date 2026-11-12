#pragma once

#include <cmath>
#include <algorithm>

class HysteresisDeadband {
public:
    /**
     * Configures the thresholds for the filter.
     * @param activate_threshold The absolute value the input must exceed to trigger a non-zero output.
     * @param release_threshold The absolute value the input must fall below to return the output to zero.
     */
    void configure(double activate_threshold, double release_threshold) {
        // Validating preconditions - these can be checked or asserted
        // The real-time path assumes valid configuration.
        m_activate_threshold = std::max(0.0, activate_threshold);
        m_release_threshold = std::max(0.0, release_threshold);

        // Ensure release is not greater than activate (as per requirements)
        // If they are swapped, we clamp them to maintain valid logic.
        if (m_release_threshold > m_activate_threshold) {
            std::swap(m_activate_threshold, m_release_threshold);
        }
    }

    /**
     * Processes a single sample of the signal.
     * Hard real-time: No allocations, no exceptions, no locks, no I/O.
     */
    double process(double input) {
        const double abs_input = std::abs(input);

        if (m_is_active) {
            if (abs_input <= m_release_threshold) {
                m_is_active = false;
                return 0.0;
            }
            return input;
        } else {
            if (abs_input >= m_activate_threshold) {
                m_is_active = true;
                return input;
            }
            return 0.0;
        }
    }

private:
    double m_activate_threshold = 0.0;
    double m_release_threshold = 0.0;
    bool m_is_active = false;
};
