#pragma once

#include <cmath>
#include <algorithm>

class DynamicRangeCompressor {
public:
    void configure(float threshold_db, float ratio, float attack_ms, float release_ms, float sample_rate) {
        m_threshold_db = threshold_db;
        m_ratio = std::max(1.0f, ratio);
        m_sample_rate = std::max(1.0f, sample_rate);

        // Calculate coefficients
        // target_gain = 10^((1/ratio - 1) * (input_db - threshold_db))
        // We use the standard feed-forward formula:
        // Gain(dB) = Threshold_dB + (1/ratio) * (Input_dB - Threshold_dB)
        // Actually, the standard formula for gain reduction is:
        // If Input_dB > Threshold_dB, Gain_dB = Threshold_dB + (Input_dB - Threshold_dB) / ratio
        // The multiplier is 10^((Gain_dB - Input_dB) / 20)
        
        // Pre-calculate time constants
        // alpha = 1 - e^(-1 / (time_const_samples))
        // A common high-quality approach for smoothing:
        float attack_coeff = std::exp(-1.0f / (m_sample_rate * (attack_ms / 1000.0f)));
        float release_coeff = std::exp(-1.0f / (m_sample_rate * (release_ms / 1000.0f)));

        // Since we need coefficients that are 1 - alpha, but often simplified:
        // We'll use the standard 1 - exp(-1 / (fs * time))
        // However, to make it simpler and consistent with standard DSP:
        m_attack_coeff = 1.0f - std::exp(-1.0f / (m_sample_rate * (attack_ms / 1000.0f)));
        m_release_coeff = 1.0f - std::exp(-1.0f / (m_sample_rate * (release_ms / 1000.0f)));

        m_current_gain = 1.0f;
    }

    float process(float input) {
        float abs_input = std::abs(input);
        // Convert to dBFS. Use a floor to avoid -inf.
        float input_db = 20.0f * std::log10(abs_input + 1e-12f);

        float target_gain = 1.0f;
        if (input_db > m_threshold_db) {
            // Amount over threshold
            float over_db = input_db - m_threshold_db;
            // The gain reduction is 1/ratio for the amount over
            // Gain_dB = (over_db / m_ratio) - over_db = over_db * (1/m_ratio - 1)
            // Wait, the standard formula is: Gain_dB = Threshold_dB + (Input_dB - Threshold_dB) / Ratio
            // The reduction is relative to the input level. 
            // Output_dB = Threshold_dB + (Input_dB - Threshold_dB) / Ratio
            // Reduction_dB = Input_dB - Output_dB = (Input_dB - Threshold_dB) * (1 - 1/Ratio)
            float reduction_db = over_db * (1.0f - (1.0f / m_ratio));
            target_gain = std::pow(10.0f, -reduction_db / 20.0f);
        }

        // Smoothing
        // If target_gain is less than current, we are "attacking" (gain is decreasing)
        // If target_gain is greater than current, we are "releasing" (gain is increasing)
        float coeff = (target_gain < m_current_gain) ? m_attack_coeff : m_release_coeff;
        
        // Smoothing filter: y[n] = y[n-1] + coeff * (x[n] - y[n-1])
        m_current_gain += coeff * (target_gain - m_current_gain);

        return input * m_current_gain;
    }

private:
    float m_threshold_db = -20.0f;
    float m_ratio = 1.0f;
    float m_attack_coeff = 0.01f;
    float m_release_coeff = 0.01f;
    float m_sample_rate = 44100.0f;
    float m_current_gain = 1.0f;
};
