#pragma once
#include <cstddef>
#include <vector>
#include <cmath>
#include <algorithm>

class StereoMixer {
public:
    void configure(size_t max_channels, size_t block_size) {
        max_ch_ = max_channels;
        block_size_ = block_size;
        // Pre-allocate memory for gains
        gains_l_.resize(max_channels);
        gains_r_.resize(max_channels);
        // Initialize with 0.0 to avoid garbage
        std::fill(gains_l_.begin(), gains_l_.end(), 0.0f);
        std::fill(gains_r_.begin(), gains_r_.end(), 0.0f);
    }

    void set_pan(size_t channel_idx, float pan) {
        if (channel_idx >= max_ch_) return;
        
        // Clamp pan to [-1.0, 1.0]
        float p = std::max(-1.0f, std::min(1.0f, pan));
        
        // Calculate angle in radians
        // pan -1.0 -> 0, pan 0 -> pi/4, pan 1.0 -> pi/2
        float theta = (p + 1.0f) * (static_cast<float>(M_PI) / 4.0f);
        
        gains_l_[channel_idx] = std::cos(theta);
        gains_r_[channel_idx] = std::sin(theta);
    }

    void process(const float* const* input_buffers, size_t num_channels, 
                  float* left_out, float* right_out, size_t num_samples) {
        // Safety checks (non-throwing)
        if (num_channels > max_ch_ || num_samples > block_size_) return;

        // Clear output buffers
        for (size_t i = 0; i < num_samples; ++i) {
            left_out[i] = 0.0f;
            right_out[i] = 0.0f;
        }

        // Mix buffers
        for (size_t ch = 0; ch < num_channels; ++ch) {
            const float* src = input_buffers[ch];
            float gL = gains_l_[ch];
            float gR = gains_r_[ch];

            for (size_t i = 0; i < num_samples; ++i) {
                left_out[i] += src[i] * gL;
                right_out[i] += src[i] * gR;
            }
        }
    }

private:
    size_t max_ch_ = 0;
    size_t block_size_ = 0;
    std::vector<float> gains_l_;
    std::vector<float> gains_r_;
};
