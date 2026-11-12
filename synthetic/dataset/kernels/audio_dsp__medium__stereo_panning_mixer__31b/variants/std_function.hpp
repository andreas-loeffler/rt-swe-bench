#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include <functional>

class StereoMixer {
    struct PanGains {
        float left = 0.70710678118f;
        float right = 0.70710678118f;
    };

    size_t m_max_channels = 0;
    size_t m_block_size = 0;
    std::vector<PanGains> m_channel_gains;

    void apply_mixing_logic(const float* src, float gL, float gR, float* left_out, float* right_out, size_t num_samples) {
        // Use a lambda to encapsulate the sample-processing loop
        // Capture a large amount of data to ensure std::function allocates on the heap
        struct Context {
            float left_gain;
            float right_gain;
            size_t samples;
            const float* input;
            float* output_l;
            float* output_r;
            double padding[8]; 
        } ctx { gL, gR, num_samples, src, left_out, right_out, {0} };

        std::function<void()> mix_op = [ctx]() {
            for (size_t i = 0; i < ctx.samples; ++i) {
                ctx.output_l[i] += ctx.input[i] * ctx.left_gain;
                ctx.output_r[i] += ctx.input[i] * ctx.right_gain;
            }
        };

        mix_op();
    }

public:
    void configure(size_t max_channels, size_t block_size) {
        m_max_channels = max_channels;
        m_block_size = block_size;
        m_channel_gains.resize(max_channels);
    }

    void set_pan(size_t channel_idx, float pan) {
        if (channel_idx >= m_max_channels) return;
        
        float clamped_pan = std::clamp(pan, -1.0f, 1.0f);
        float theta = (clamped_pan + 1.0f) * (3.14159265358979323846f / 4.0f);
        
        m_channel_gains[channel_idx].left = std::cos(theta);
        m_channel_gains[channel_idx].right = std::sin(theta);
    }

    void process(const float* const* input_buffers, size_t num_channels, float* left_out, float* right_out, size_t num_samples) {
        if (num_channels > m_max_channels || num_samples > m_block_size) return;

        for (size_t i = 0; i < num_samples; ++i) {
            left_out[i] = 0.0f;
            right_out[i] = 0.0f;
        }

        for (size_t ch = 0; ch < num_channels; ++ch) {
            apply_mixing_logic(
                input_buffers[ch], 
                m_channel_gains[ch].left, 
                m_channel_gains[ch].right, 
                left_out, 
                right_out, 
                num_samples
            );
        }
    }
};
