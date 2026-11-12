#pragma once
#include <algorithm>
#include <vector>

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
        
        update_peaks(clamped, dt);
    }

    AnalysisResult get_peaks() const {
        return { m_max_peak, m_min_peak };
    }

private:
    std::vector<float> calculate_new_peaks(float clamped, float dt) {
        float next_max = m_max_peak;
        float next_min = m_min_peak;

        if (clamped > next_max) {
            next_max = clamped;
        } else {
            next_max -= (next_max - clamped) * (dt / m_decay_time);
        }

        if (clamped < next_min) {
            next_min = clamped;
        } else {
            next_min += (clamped - next_min) * (dt / m_decay_time);
        }

        return { next_max, next_min };
    }

    void update_peaks(float clamped, float dt) {
        std::vector<float> peaks = calculate_new_peaks(clamped, dt);
        m_max_peak = peaks[0];
        m_min_peak = peaks[1];
    }

    float m_decay_time{1.0f};
    float m_min_sat{0.0f};
    float m_max_sat{0.0f};
    float m_max_peak{0.0f};
    float m_min_peak{0.0f};
};
