#pragma once

#include <cmath>
#include <algorithm>
#include <vector>

namespace detail {
    class GainCalculator {
    public:
        float compute_target(float input_abs, float threshold, float ratio) {
            if (input_abs <= threshold) {
                return 1.0f;
            }

            // Use a temporary buffer to store parameters for the calculation
            // to avoid polluting the local stack and keep logic modular.
            std::vector<float> params = { threshold, input_abs, ratio };
            
            float ratio_factor = 1.0f - (1.0f / params[2]);
            return std::pow(params[0] / params[1], ratio_factor);
        }
    };

    class GainSmoother {
    public:
        void configure(float attack_coeff, float release_coeff) {
            attack_ = attack_coeff;
            release_ = release_coeff;
            current_gain_ = 1.0f;
        }

        float next(float target_gain) {
            float coeff = (target_gain < current_gain_) ? attack_ : release_;
            current_gain_ = current_gain_ + coeff * (target_gain - current_gain_);
            return current_gain_;
        }

    private:
        float attack_ = 0.0f;
        float release_ = 0.0f;
        float current_gain_ = 1.0f;
    };
}

class DynamicRangeCompressor {
public:
    void configure(float threshold_db, float ratio, float attack_ms, float release_ms, float sample_rate) {
        threshold_lin_ = std::pow(10.0f, threshold_db / 20.0f);
        ratio_ = ratio;

        float attack_time_s = attack_ms / 1000.0f;
        float release_time_s = release_ms / 1000.0f;

        smoother_.configure(
            1.0f - std::exp(-1.0f / (attack_time_s * sample_rate)),
            1.0f - std::exp(-1.0f / (release_time_s * sample_rate))
        );
    }

    float process(float input) {
        float abs_input = std::abs(input);
        float target_gain = calculator_.compute_target(abs_input, threshold_lin_, ratio_);
        float current_gain = smoother_.next(target_gain);
        return input * current_gain;
    }

private:
    float threshold_lin_ = 0.0f;
    float ratio_ = 1.0f;
    detail::GainCalculator calculator_;
    detail::GainSmoother smoother_;
};
