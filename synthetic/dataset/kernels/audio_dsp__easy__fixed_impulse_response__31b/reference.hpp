#pragma once

#include <vector>
#include <span>

class ImpulseResponseConvolver {
public:
    void configure(std::span<const float> coefficients) {
        coeffs_ = std::vector<float>(coefficients.begin(), coefficients.end());
        buffer_.assign(coeffs_.size(), 0.0f);
        write_pos_ = 0;
    }

    float process(float input) {
        buffer_[write_pos_] = input;
        
        float sum = 0.0f;
        const size_t size = coeffs_.size();
        for (size_t i = 0; i < size; ++i) {
            // Index backward from the most recent sample (write_pos_)
            size_t sample_idx = (write_pos_ + size - i) % size;
            sum += coeffs_[i] * buffer_[sample_idx];
        }

        write_pos_ = (write_pos_ + 1) % size;
        return sum;
    }

private:
    std::vector<float> coeffs_;
    std::vector<float> buffer_;
    size_t write_pos_ = 0;
};
