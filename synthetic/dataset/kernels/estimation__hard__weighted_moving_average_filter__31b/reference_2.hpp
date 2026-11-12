#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

enum class WeightMode {
    Linear,
    Gaussian
};

class WeightedMovingAverage {
private:
    struct Weights {
        std::vector<float> mut_weights; // Pre-calculated weights
        size_t size;
    };

    size_t m_window_size = 0;
    WeightMode m_mode = WeightMode::Linear;
    float m_sigma = 1.0f;

    std::vector<float> m_buffer;
    std::vector<float> m_weights;
    
    size_t m_write_idx = 0;
    size_t m_count = 0;
    float m_total_weight_sum = 0.0f;

    void clear_buffers() {
        m_buffer.clear();
        m_weights.clear();
        m_write_idx = 0;
        m_count = 0;
        m_total_weight_sum = 0.0f;
    }

public:
    void configure(size_t window_size, WeightMode mode, float sigma = 1.0f) {
        m_window_size = window_size;
        m_mode = mode;
        m_sigma = sigma;

        m_buffer.assign(m_window_size, 0.0f);
        m_weights.assign(m_window_size, 0.0f);

        // Weights are calculated relative to the "newest" sample at index 0
        // but are stored in a way that we can map them easily.
        // Let's store them such that weights[i] is weight for sample at (current_idx - i)
        for (size_t i = 0; i < m_window_size; ++i) {
            float w = 0.0f;
            if (m_mode == WeightMode::Linear) {
                // Weight decreases linearly from most recent (max) to oldest (min)
                // The prompt says: "most recent (weight = W) to oldest (weight = W - (N-1))"
                // Since W is arbitrary, we use a normalized approach where the oldest is 1.0
                // and the newest is m_window_size.
                w = static_cast<float>(m_window_size - i);
            } else {
                // Gaussian: weights[i] = exp(- (i^2) / (2 * sigma^2))
                // i is the distance from the most recent sample.
                float dist = static_cast<float>(i);
                w = std::exp(-(dist * dist) / (2.0f * m_sigma * m_sigma));
            }
            m_weights[i] = w;
        }
        reset();
    }

    void push(float sample) {
        m_buffer[m_write_idx] = sample;
        m_write_idx = (m_write_idx + 1) % m_window_size;
        if (m_count < m_window_size) {
            m_count++;
        }
    }

    float process() {
        if (m_count == 0) return 0.0f;

        float weighted_sum = 0.0f;
        float current_total_weight = 0.0f;

        // The buffer stores elements in circular order. 
        // The most recent sample is at (m_write_idx - 1 + m_window_size) % m_window_size.
        // We iterate from the most recent back to the oldest.
        for (size_t i = 0; i < m_count; ++i) {
            // distance from newest: 0 is newest, 1 is one back, etc.
            size_t idx = (m_write_idx + m_window_size - 1 - i) % m_window_size;
            float weight = m_weights[i];
            weighted_sum += m_buffer[idx] * weight;
            current_total_weight += weight;
        }

        if (current_total_weight > 0.0f) {
            return weighted_sum / current_total_weight;
        }
        return 0.0f;
    }

    void reset() {
        std::fill(m_buffer.begin(), m_buffer.end(), 0.0f);
        m_write_idx = 0;
        m_count = 0;
        m_total_weight_sum = 0.0f;
    }
};
