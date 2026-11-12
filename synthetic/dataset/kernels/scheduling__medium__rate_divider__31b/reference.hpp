#pragma once
#include <cstdint>

class RateDivider {
public:
    void configure(double master_rate, double target_rate) {
        // Target rate must be <= master rate to be representable by sample counting
        m_increment = target_rate / master_rate;
        m_phase = 0.0;
    }

    bool process() {
        m_phase += m_increment;
        if (m_phase >= 1.0) {
            m_phase -= 1.0;
            return true;
        }
        return false;
    }

    double get_phase() const {
        return m_phase;
    }

private:
    double m_increment = 0.0;
    double m_phase = 0.0;
};
