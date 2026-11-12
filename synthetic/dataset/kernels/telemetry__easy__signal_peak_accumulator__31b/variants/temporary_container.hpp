#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class SignalPeakAccumulator {
public:
    void configure(size_t batch_size) {
        m_batch_size = batch_size;
        m_current_peak = 0.0f;
        m_count = 0;
    }

    float process(float sample) noexcept {
        m_samples.push_back(sample);
        m_count++;

        if (m_count >= m_batch_size) {
            std::vector<float> magnitudes;
            magnitudes.reserve(m_samples.size());
            for (float s : m_samples) {
                magnitudes.push_back(std::abs(s));
            }

            float peak = 0.0f;
            for (float mag : magnitudes) {
                if (mag > peak) {
                    peak = mag;
                }
            }

            m_samples.clear();
            m_count = 0;
            return peak;
        }

        return 0.0f;
    }

private:
    size_t m_batch_size = 0;
    size_t m_count = 0;
    float m_current_peak = 0.0f;
    std::vector<float> m_samples;
};
