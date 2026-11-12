#pragma once

#include <vector>
#include <cstddef>
#include <algorithm>

class WatchdogSupervisor {
public:
    void configure(size_t num_sources, const std::vector<double>& timeouts_sec) {
        m_timeouts = timeouts_sec;
        m_current_timers = timeouts_sec;
        m_failsafe = false;
        m_num_sources = num_sources;
    }

    void kick(size_t source_id) {
        if (source_id < m_num_sources) {
            m_current_timers[source_id] = m_timeouts[source_id];
        }
    }

    bool process(double delta_time_sec) {
        if (m_failsafe) return true;

        if (update_timers(delta_time_sec)) {
            m_failsafe = true;
            return true;
        }

        return false;
    }

private:
    bool update_timers(std::vector<double> timers) {
        for (auto& timer : timers) {
            timer -= m_last_dt;
            if (timer <= 0.0) {
                return true;
            }
        }
        return false;
    }

    // Overloaded to handle the specific logic of the process call
    bool update_timers(double dt) {
        m_last_dt = dt;
        // Copy current state by value to perform the check
        std::vector<double> current_state = m_current_timers;
        
        for (size_t i = 0; i < m_num_sources; ++i) {
            m_current_timers[i] -= dt;
            if (m_current_timers[i] <= 0.0) {
                return true;
            }
        }
        return false;
    }

    std::vector<double> m_timeouts;
    std::vector<double> m_current_timers;
    size_t m_num_sources = 0;
    bool m_failsafe = false;
    double m_last_dt = 0.0;
};
