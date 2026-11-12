#pragma once

#include <vector>
#include <cstddef>

class WatchdogSupervisor {
public:
    void configure(size_t num_sources, const std::vector<double>& timeouts_sec) {
        m_timeouts.assign(timeouts_sec.begin(), timeouts_sec.end());
        m_current_timers.assign(timeouts_sec.begin(), timeouts_sec.end());
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

        for (size_t i = 0; i < m_num_sources; ++i) {
            m_current_timers[i] -= delta_time_sec;
            if (m_current_timers[i] <= 0.0) {
                m_failsafe = true;
                return true;
            }
        }
        return false;
    }

private:
    std::vector<double> m_timeouts;
    std::vector<double> m_current_timers;
    size_t m_num_sources = 0;
    bool m_failsafe = false;
};
