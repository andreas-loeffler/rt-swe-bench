#pragma once

#include <cmath>
#include <algorithm>
#include <vector>

namespace detail {
    struct SmoothingParams {
        float attack;
        float release;
    };

    class GainSmoother {
    public:
        void configure(SmoothingParams params) {
            params_ = params;
            current_gain_ = 1.0f;
        }

        float next(float target_gain) {
            float coeff = (target_gain < current_gain_) ? params_.attack : params_.release;
            current_gain_ = current_gain_ + coeff * (target_gain - current_gain_);
            return current_gain_;
        }

    private:
        SmoothingParams params_{0.0f, 0.0f};
        float current_gain_ = 1.0f;
    };

    std::vector<float> calculate_coeffs(float attack_ms, float release_ms, float sample_rate) {
        float attack_time_s = attack_ms / 1000.0f;
        float release_time_s = release_ms / 1000.0f;
        
        std::vector<float> coeffs;
        coeffs.push_back(1.0f - std::exp(-1.0f / (attack_time_s * sample_rate)));
        coeffs.push_back(1.0f - std::exp(-1.0f / (release_time_s * sample_rate)));
        return coeffs;
    }
}

class DynamicRangeCompressor {
public:
    void configure(float threshold_db, float ratio, float attack_ms, float release_ms, float sample_rate) {
        threshold_lin_ = std::pow(10.0f, threshold_db / 20.0f);
        ratio_ = ratio;
        
        attack_ms_ = attack_ms;
        release_ms_ = release_ms;
        sample_rate_ = sample_rate;
    }

    float process(float input) {
        if (sample_rate_ <= 0.0f) return input;

        float abs_input = std::abs(input);
        float target_gain = 1.0f;

        if (abs_input > threshold_lin_) {
            float ratio_factor = 1.0f - (1.0f / ratio_);
            target_gain = std::pow(threshold_lin_ / abs_input, ratio_factor);
        }

        if (!smoother_initialized_) {
            auto coeffs = detail::calculate_coeffs(attack_ms_, release_ms_, sample_rate_);
            smoother_.configure({coeffs[0], coeffs[1]});
            smoother_initialized_ = true;
        }

        float current_gain = smoother_.next(target_gain);
        return input * current_gain;
    }

private:
    float threshold_lin_ = 0.0f;
    float ratio_ = 1.0f;
    float attack_ms_ = 0.0f;
    float release_ms_ = 0.0f;
    float sample_rate_ = 0.0f;
    bool smoother_initialized_ = false;
    detail::GainSmoother smoother_;
};
