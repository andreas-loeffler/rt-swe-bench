#pragma once
#include <vector>
#include <numeric>

class FirFilter {
public:
    void configure(const std::vector<float>& coefficients) {
        coeffs_ = coefficients;
        history_.assign(coeffs_.size(), 0.0f);
    }

    float process(float input) {
        // Shift history
        for (size_t i = history_.size() - 1; i > 0; --i) {
            history_[i] = history_[i - 1];
        }
        history_[0] = input;

        float output = 0.0f;
        for (size_t i = 0; i < coeffs_.size(); ++i) {
            output += history_[i] * coeffs_[i];
        }
        return output;
    }

private:
    std::vector<float> coeffs_;
    std::vector<float> history_;
};
