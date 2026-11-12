#pragma once

#include <cstdint>

class SoftwareDebouncer {
public:
    void configure(uint32_t required_cycles) {
        m_required_cycles = required_cycles;
        m_stable_state = false;
        m_counter = 0;
    }

    bool process(bool input) {
        if (input == m_stable_state) {
            m_counter = 0;
        } else {
            m_counter++;
            if (m_counter >= m_required_cycles) {
                m_stable_state = input;
                m_counter = 0;
            }
        }
        return m_stable_state;
    }

private:
    uint32_t m_required_cycles{0};
    bool m_stable_state{false};
    uint32_t m_counter{0};
};
