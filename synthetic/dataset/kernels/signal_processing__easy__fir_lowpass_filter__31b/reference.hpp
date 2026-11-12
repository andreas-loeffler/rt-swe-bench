#pragma once
#include <vector>

class FirLowpassFilter {
public:
    void configure(std::vector<float> coefficients) {
        coeffs_ = std::move(coefficients);
        history_.assign(coeffs_.size(), 0.0f);
        head_ = 0;
        size_ = history_.size();
    }

    float process(float sample) {
        history_[head_] = sample;
        
        float accumulation = 0.0f;
        for (size_t i = 0; i < size_; ++i) {
            // Access coefficients in order, history in reverse from head
            size_t idx = (head_ + size_ - i) % size_;
            accumulation += coeffs_[i] * history_[idx];
        }
        
        head_ = (head_ + 1) % size_;
        return accumulation;
    }

private:
    std::vector<float> coeffs_;
    std::vector<float> history_;
    size_t head_ = 0;
    size_t size_ = 0;
};
