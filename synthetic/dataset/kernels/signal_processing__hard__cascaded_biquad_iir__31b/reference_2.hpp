#pragma once
#include <cstddef>
#include <vector>
#include <cmath>
#include <algorithm>

/**
 * Implementation details:
 * Uses Direct Form II Transposed (DF2T) for the Biquad sections.
 * 
 * Difference Equation for DF2T:
 * y[n] = b0*x[n] + s1[n-1]
 * s1[n]   = b1*x[n] - a1*y[n] + s2[n-1]
 * s2[n]   = b2*x[n] - a2*y[n]
 * 
 * Rearranged for efficient processing:
 * y[n] = b0*x[n] + s1
 * s1 = b1*x[n] - a1*y[n] + s2
 * s2 = b2*x[n] - a2*y[n]
 */

class BiquadSection {
public:
    struct Coefficients {
        float b0, b1, b2, a1, a2;
    };

    BiquadSection() : coeffs{1.0f, 0.0f, 0.0f, 0.0f, 0.0f}, s1{0.0f}, s2{0.0f} {}

    void set_coeffs(float b0, float b1, float b2, float a1, float a2) {
        coeffs = {b0, b1, b2, a1, a2};
    }

    void reset() {
        s1 = 0.0f;
        s2 = 0.0f;
    }

    float process(float x) {
        float y = coeffs.b0 * x + s1;
        s1 = coeffs.b1 * x - coeffs.a1 * y + s2;
        s2 = coeffs.b2 * x - coeffs.a2 * y;
        return y;
    }

    bool stable() const {
        // Stability for a Biquad:
        // The roots of z^2 + a1*z + a2 = 0 must be inside the unit circle.
        // Conditions: 
        // 1. |a2| < 1
        // 2. |a1| < 1 + a2 (for a2 > 0) or a1 > -(1 + a2) (for a2 < 0)
        // Combined: |a1| < 1 + a2 AND |a2| < 1
        return (std::abs(coeffs.a2) < 1.0f) && 
               (coeffs.a1 < 1.0f + coeffs.a2) && 
               (coeffs.a1 > -1.0f - coeffs.a2);
    }

private:
    Coefficients coeffs;
    float s1; // State register 1
    float s2; // State register 2
};

class CascadedBiquadIIR {
public:
    void configure(size_t num_sections, float sample_rate) {
        num_sections_ = num_sections;
        sample_rate_ = sample_rate;
        sections_.clear();
        sections_.reserve(num_sections_);
        for (size_t i = 0; i < num_sections_; ++i) {
            sections_.emplace_back();
        }
    }

    void set_coefficients(size_t section_idx, float b0, float b1, float b2, float a1, float a2) {
        if (section_idx < sections_.size()) {
            sections_[section_idx].set_coeffs(b0, b1, b2, a1, a2);
        }
    }

    float process(float input) {
        float out = input;
        for (auto& section : sections_) {
            out = section.process(out);
        }
        return out;
    }

    void reset() {
        for (auto& section : sections_) {
            section.reset();
        }
    }

    bool is_stable() const {
        for (const auto& section : sections_) {
            if (!section.stable()) return false;
        }
        return true;
    }

private:
    size_t num_sections_ = 0;
    float sample_rate_ = 0.0f;
    std::vector<BiquadSection> sections_;
};
