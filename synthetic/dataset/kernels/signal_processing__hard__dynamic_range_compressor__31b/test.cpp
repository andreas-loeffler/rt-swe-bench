#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    DynamicRangeCompressor compressor;
    
    // Test parameters
    const float threshold_db = -6.0f; // ~0.5f
    const float ratio = 4.0f;
    const float attack_ms = 1.0f;
    const float release_ms = 10.0f;
    const float sample_rate = 44100.0f;

    rtk::guarded_void(run, [&] {
        compressor.configure(threshold_db, ratio, attack_ms, release_ms, sample_rate);
    });

    // Independent Reference implementation for testing logic
    auto get_expected = [&](float input, float& current_gain) {
        float threshold_lin = std::pow(10.0f, threshold_db / 20.0f);
        float attack_coeff = 1.0f - std::exp(-1.0f / ((attack_ms / 1000.0f) * sample_rate));
        float release_coeff = 1.0f - std::exp(-1.0f / ((release_ms / 1000.0f) * sample_rate));
        
        float abs_input = std::abs(input);
        float target_gain = 1.0f;
        if (abs_input > threshold_lin) {
            target_gain = std::pow(threshold_lin / abs_input, 1.0f - (1.0f / ratio));
        }
        
        float coeff = (target_gain < current_gain) ? attack_coeff : release_coeff;
        current_gain = current_gain + coeff * (target_gain - current_gain);
        return input * current_gain;
    };

    float ref_gain = 1.0f;
    
    // 1. Test Under-threshold (no compression)
    for (int i = 0; i < 200; ++i) {
        float input = 0.1f; 
        float expected = get_expected(input, ref_gain);
        float actual = rtk::guarded(run, [&] { return compressor.process(input); });
        rtk::expect(run, rtk::near(actual, expected, 1e-6), "Under-threshold");
    }

    // 2. Test Over-threshold (compression trigger)
    for (int i = 0; i < 200; ++i) {
        float input = 0.9f; 
        float expected = get_expected(input, ref_gain);
        float actual = rtk::guarded(run, [&] { return compressor.process(input); });
        rtk::expect(run, rtk::near(actual, expected, 1e-6), "Over-threshold");
    }

    // 3. Test Recovery (release phase)
    for (int i = 0; i < 200; ++i) {
        float input = 0.1f; 
        float expected = get_expected(input, ref_gain);
        float actual = rtk::guarded(run, [&] { return compressor.process(input); });
        rtk::expect(run, rtk::near(actual, expected, 1e-6), "Recovery");
    }

    // 4. Test Bipolar signals
    for (int i = 0; i < 100; ++i) {
        float input = (i % 2 == 0) ? 0.8f : -0.8f;
        float expected = get_expected(input, ref_gain);
        float actual = rtk::guarded(run, [&] { return compressor.process(input); });
        rtk::expect(run, rtk::near(actual, expected, 1e-6), "Bipolar");
    }

    return rtk::report(run);
}
