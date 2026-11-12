#pragma once

#include <cstdint>
#include <vector>

class SoftwareDebouncer {
public:
    void configure(uint32_t required_cycles) {
        m_required_cycles = required_cycles;
        m_stable_state = false;
        m_counter = 0;
    }

    bool process(bool input) {
        std::vector<bool> state_buffer = { m_stable_state };
        
        if (input == state_buffer[0]) {
            m_counter = 0;
        } else {
            m_counter++;
            if (m_counter >= m_required_cycles) {
                state_buffer[0] = input;
                m_counter = 0;
            }
        }
        
        m_stable_state = state_buffer[0];
        return m_stable_state;
    }

private:
    uint32_t m_required_cycles{0};
    bool m_stable_state{false};
    uint32_t m_counter{0};
};
