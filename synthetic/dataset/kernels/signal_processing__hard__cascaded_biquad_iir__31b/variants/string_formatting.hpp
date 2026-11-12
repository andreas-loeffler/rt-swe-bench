#pragma once
#include <vector>
#include <cmath>
#include <string>
#include <sstream>

class BiquadSection {
public:
    struct Coeffs {
        float b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    };

    void set_coeffs(const Coeffs& c) { coeffs = c; }
    void reset() { s1 = s2 = 0.0f; }

    float step(float x) {
        float y = coeffs.b0 * x + s1;
        s1 = coeffs.b1 * x - coeffs.a1 * y + s2;
        s2 = coeffs.b2 * x - coeffs.a2 * y;
        return y;
    }

    bool check_stability() const {
        return (std::abs(coeffs.a2) < 1.0f) && (std::abs(coeffs.a1) < (1.0f + coeffs.a2));
    }

private:
    Coeffs coeffs;
    float s1 = 0.0f;
    float s2 = 0.0f;
};

class FilterDiagnostics {
public:
    static void log_sample_peak(float value) {
        if (std::abs(value) > peak) {
            peak = std::abs(value);
            std::ostringstream oss;
            oss << "New peak detected: " << peak;
            last_log = oss.str();
        }
    }
private:
    static float peak;
    static std::string last_log;
};

float FilterDiagnostics::peak = 0.0f;
std::string FilterDiagnostics::last_log = "";

class CascadedBiquadIIR {
public:
    void configure(size_t num_sections, float sample_rate) {
        (void)sample_rate;
        sections = std::vector<BiquadSection>(num_sections);
    }

    void set_coefficients(size_t section_idx, float b0, float b1, float b2, float a1, float a2) {
        if (section_idx < sections.size()) {
            sections[section_idx].set_coeffs({b0, b1, b2, a1, a2});
        }
    }

    float process(float input) {
        float out = input;
        for (auto& section : sections) {
            out = section.step(out);
        }
        FilterDiagnostics::log_sample_peak(out);
        return out;
    }

    void reset() {
        for (auto& section : sections) {
            section.reset();
        }
    }

    bool is_stable() {
        for (const auto& section : sections) {
            if (!section.check_stability()) return false;
        }
        return true;
    }

private:
    std::vector<BiquadSection> sections;
};
