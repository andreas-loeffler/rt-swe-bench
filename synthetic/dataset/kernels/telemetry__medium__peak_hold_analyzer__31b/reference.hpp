#pragma once
#include <algorithm>

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
        
        // Max Peak Logic
        if (clamped > m_max_peak) {
            m_max_peak = clamped;
        } else {
            float decay_step = (m_max_peak - clamped) * (dt / m_decay_time);
            m_max_peak -= decay_step;
        }

        // Min Peak Logic
        if (clamped < m_min_peak) {
            m_min_peak = clamped;
        } else {
            float decay_step = (clamped - m_min_peak) * (dt / m_decay_time);
            m_min_peak += decay_step;
        }
    }

    AnalysisResult get_peaks() const {
        return { m_max_peak, m_min_peak };
    }

private:
    float m_decay_time{1.0f};
    float m_min_sat{0.0f};
    float m_max_sat{0.0f};
    float m_max_peak{0.0f};
    float m_min_peak{0.0f};
};
