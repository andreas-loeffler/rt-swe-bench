#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class StereoMixer {
    struct PanGains {
        float left = 0.70710678118f;
        float right = 0.70710678118f;
    };

    size_t m_max_channels = 0;
    size_t m_block_size = 0;
    std::vector<PanGains> m_channel_gains;

public:
    void configure(size_t max_channels, size_t block_size) {
        m_max_channels = max_channels;
        m_block_size = block_size;
        m_channel_gains.resize(max_channels);
    }

    void set_pan(size_t channel_idx, float pan) {
        if (channel_idx >= m_max_channels) return;
        
        // Constant power panning law
        // pan -1.0 -> 0 rad, pan 0.0 -> pi/4 rad, pan 1.0 -> pi/2 rad
        float clamped_pan = std::clamp(pan, -1.0f, 1.0f);
        float theta = (clamped_pan + 1.0f) * (3.14159265358979323846f / 4.0f);
        
        m_channel_gains[channel_idx].left = std::cos(theta);
        m_channel_gains[channel_idx].right = std::sin(theta);
    }

    void process(const float* const* input_buffers, size_t num_channels, float* left_out, float* right_out, size_t num_samples) {
        if (num_channels > m_max_channels || num_samples > m_block_size) return;

        // Clear output buffers
        for (size_t i = 0; i < num_samples; ++i) {
            left_out[i] = 0.0f;
            right_out[i] = 0.0f;
        }

        // Mix channels
        for (size_t ch = 0; ch < num_channels; ++ch) {
            const float* src = input_buffers[ch];
            const float gL = m_channel_gains[ch].left;
            const float gR = m_channel_gains[ch].right;

            for (size_t i = 0; i < num_samples; ++i) {
                left_out[i] += src[i] * gL;
                right_out[i] += src[i] * gR;
            }
        }
    }
};
