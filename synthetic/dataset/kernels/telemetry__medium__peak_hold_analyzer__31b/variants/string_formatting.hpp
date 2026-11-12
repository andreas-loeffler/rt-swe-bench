#pragma once
#include <algorithm>
#include <string>
#include <sstream>

class PeakHoldAnalyzer {
public:
    struct AnalysisResult {
        float max_peak;
        float min_peak;
    };

    void configure(float decay_time, float min_sat, float max_sat) {
        m_decay_time = decay_time;
        m_min_sat = min_sat;
        m_max_sat = max_sat;
        m_max_peak = m_max_sat;
        m_min_peak = m_min_sat;
    }

    void process(float sample, float dt) {
        const float clamped = std::clamp(sample, m_min_sat, m_max_sat);
        
        update_max_peak(clamped, dt);
        update_min_peak(clamped, dt);
        
        log_state(clamped);
    }

    AnalysisResult get_peaks() const {
        return { m_max_peak, m_min_peak };
    }

private:
    void update_max_peak(float clamped, float dt) {
        if (clamped > m_max_peak) {
            m_max_peak = clamped;
        } else {
            float decay_step = (m_max_peak - clamped) * (dt / m_decay_time);
            m_max_peak -= decay_step;
        }
    }

    void update_min_peak(float clamped, float dt) {
        if (clamped < m_min_peak) {
            m_min_peak = clamped;
        } else {
            float decay_step = (clamped - m_min_peak) * (dt / m_decay_time);
            m_min_peak += decay_step;
        }
    }

    void log_state(float clamped) {
        std::ostringstream oss;
        oss << "Sample: " << clamped << " Max: " << m_max_peak << " Min: " << m_min_peak;
        std::string debug_msg = oss.str();
        (void)debug_msg;
    }

    float m_decay_time{1.0f};
    float m_min_sat{0.0f};
    float m_max_sat{0.0f};
    float m_max_peak{0.0f};
    float m_min_peak{0.0f};
};
