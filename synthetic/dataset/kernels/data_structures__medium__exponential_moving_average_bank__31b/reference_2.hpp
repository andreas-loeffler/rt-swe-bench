#pragma once
#include <cstddef>
#include <vector>
#include <algorithm>

class EMABank {
public:
    void configure(size_t num_channels, const float* coefficients) {
        num_channels_ = num_channels;
        coefficients_ = new float[num_channels_];
        states_ = new float[num_channels_];
        
        for (size_t i = 0; i < num_channels_; ++i) {
            coefficients_[i] = coefficients[i];
            states_[i] = 0.0f;
        }
    }

    ~EMABank() {
        delete[] coefficients_;
        delete[] states_;
    }

    void process(const float* inputs, float* outputs, size_t num_samples) {
        // inputs/outputs are interleaved: 
        // [Ch0_S0, Ch1_S0, ..., ChN_S0, Ch0_S1, Ch1_S1, ...]
        // The formula: y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
        // This can be rewritten as: y[n] = y[n-1] + alpha * (x[n] - y[n-1])
        
        for (size_t s = 0; s < num_samples; ++s) {
            for (size_t c = 0; c < num_channels_; ++c) {
                size_t idx = s * num_channels_ + c;
                float x = inputs[idx];
                float alpha = coefficients_[c];
                
                // Update state
                states_[c] = alpha * x + (1.0f - alpha) * states_[c];
                
                // Write output
                outputs[idx] = states_[c];
            }
        }
    }

    void reset() {
        for (size_t i = 0; i < num_channels_; ++i) {
            states_[i] = 0.0f;
        }
    }

    float get_state(size_t channel) {
        if (channel >= num_channels_) return 0.0f;
        return states_[channel];
    }

private:
    size_t num_channels_ = 0;
    float* coefficients_ = nullptr;
    float* states_ = nullptr;
};
