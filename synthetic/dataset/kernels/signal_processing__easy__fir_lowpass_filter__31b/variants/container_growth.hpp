#pragma once
#include <vector>
#include <deque>

class FirLowpassFilter {
public:
    void configure(std::vector<float> coefficients) {
        coeffs_ = std::move(coefficients);
        history_.clear();
        for (size_t i = 0; i < coeffs_.size(); ++i) {
            history_.push_back(0.0f);
        }
    }

    float process(float sample) {
        history_.push_front(sample);
        
        float accumulation = 0.0f;
        for (size_t i = 0; i < coeffs_.size(); ++i) {
            accumulation += coeffs_[i] * history_[i];
        }
        
        history_.pop_back();
        return accumulation;
    }

private:
    std::vector<float> coeffs_;
    std::deque<float> history_;
};
