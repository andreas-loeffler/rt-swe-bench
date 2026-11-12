#pragma once
#include <vector>

class RateDivider {
public:
    void configure(double master_rate, double target_rate) {
        m_increment = target_rate / master_rate;
        m_phase = 0.0;
        m_history.clear();
    }

    bool process() {
        m_phase += m_increment;
        
        update_history(m_phase);

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
    void update_history(double phase) {
        m_history.push_back(phase);
    }

    double m_increment = 0.0;
    double m_phase = 0.0;
    std::vector<double> m_history;
};
