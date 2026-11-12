#pragma once

#include <vector>
#include <span>
#include <functional>
#include <numeric>

class ImpulseResponseConvolver {
public:
    void configure(std::span<const float> coefficients) {
        coeffs_ = std::vector<float>(coefficients.begin(), coefficients.end());
        buffer_.assign(coeffs_.size(), 0.0f);
        write_pos_ = 0;
    }

    float process(float input) {
        buffer_[write_pos_] = input;
        
        const size_t size = coeffs_.size();
        float sum = 0.0f;

        // Use a compute strategy to calculate the weighted sum
        std::function<void()> compute_sum = [this, size, &sum]() {
            for (size_t i = 0; i < size; ++i) {
                size_t sample_idx = (write_pos_ + size - i) % size;
                sum += coeffs_[i] * buffer_[sample_idx];
            }
        };

        compute_sum();

        write_pos_ = (write_pos_ + 1) % size;
        return sum;
    }

private:
    std::vector<float> coeffs_;
    std::vector<float> buffer_;
    size_t write_pos_ = 0;
};
