#pragma once

#include <cstdint>

class SoftwareDebouncer {
public:
    /**
     * @brief Configures the stability threshold.
     * 
     * @param required_cycles The number of consecutive cycles the input 
     * must be stable before the output state transitions.
     */
    void configure(uint32_t required_cycles) {
        m_required_cycles = required_cycles;
        m_current_stable_state = false;
        m_consecutive_count = 0;
    }

    /**
     * @brief Processes a single binary input sample.
     * 
     * @param input The raw input signal.
     * @return The current stable output state.
     * 
     * Real-time: No heap allocation, no locks, no exceptions, no I/O.
     */
    bool process(bool input) {
        if (input == m_current_stable_state) {
            // Input matches current stable state; reset counter
            m_consecutive_count = 0;
        } else {
            // Input differs from stable state; increment counter
            m_consecutive_count++;
            
            if (m_consecutive_count >= m_required_cycles) {
                // Threshold reached; update stable state
                m_current_stable_state = input;
                m_consecutive_count = 0;
            }
        }
        return m_current_stable_state;
    }

private:
    uint32_t m_required_cycles = 0;
    bool m_current_stable_state = false;
    uint32_t m_consecutive_count = 0;
};
