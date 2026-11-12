#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <memory>

class StereoMixer {
    struct PanGains {
        float left = 0.70710678118f;
        float right = 0.70710678118f;
    };

    size_t m_max_channels = 0;
    size_t m_block_size = 0;
    std::vector<std::shared_ptr<PanGains>> m_channel_gains;

public:
    void configure(size_t max_channels, size_t block_size) {
        m_max_channels = max_channels;
        m_block_size = block_size;
        m_channel_gains.resize(max_channels);
        for (size_t i = 0; i < max_channels; ++i) {
            m_channel_gains[i] = std::make_shared<PanGains>();
        }
    }

    void set_pan(size_t channel_idx, float pan) {
        if (channel_idx >= m_max_channels) return;
        
        float clamped_pan = std::clamp(pan, -1.0f, 1.0f);
        float theta = (clamped_pan + 1.0f) * (3.14159265358979323846f / 4.0f);
        
        auto new_gains = std::make_shared<PanGains>();
        new_gains->left = std::cos(theta);
        new_gains->right = std::sin(theta);
        
        m_channel_gains[channel_idx] = new_gains;
    }

    void process(const float* const* input_buffers, size_t num_channels, float* left_out, float* right_out, size_t num_samples) {
        if (num_channels > m_max_channels || num_samples > m_block_size) return;

        for (size_t i = 0; i < num_samples; ++i) {
            left_out[i] = 0.0f;
            right_out[i] = 0.0f;
        }

        for (size_t ch = 0; ch < num_channels; ++ch) {
            const float* src = input_buffers[ch];
            const auto& gains = m_channel_gains[ch];
            const float gL = gains->left;
            const float gR = gains->right;

            for (size_t i = 0; i < num_samples; ++i) {
                left_out[i] += src[i] * gL;
                right_out[i] += src[i] * gR;
            }
        }
    }
};
