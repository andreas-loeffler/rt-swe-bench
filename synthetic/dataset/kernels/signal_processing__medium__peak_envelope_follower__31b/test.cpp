#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    PeakEnvelopeFollower k;
    
    const float attack_s = 0.01f;
    const float release_s = 0.1f;
    const float fs = 44100.0f;
    
    k.configure(attack_s, release_s, fs);

    auto calc_expected = [&](float current, float input, float a_coeff, float r_coeff) {
        float abs_in = std::abs(input);
        if (abs_in > current) {
            return current + a_coeff * (abs_in - current);
        } else {
            return current + r_coeff * (abs_in - current);
        }
    };

    const float a_coeff = 1.0f - std::exp(-1.0f / (attack_s * fs));
    const float r_coeff = 1.0f - std::exp(-1.0f / (release_s * fs));

    float expected_env = 0.0f;

    // Test Case 1: Attack Phase (Rising signal)
    for (int i = 0; i < 250; ++i) {
        float input = 1.0f; 
        expected_env = calc_expected(expected_env, input, a_coeff, r_coeff);
        
        float result = rtk::guarded(run, [&] { return k.process(input); });
        rtk::expect(run, rtk::near(result, expected_env, 1e-6), "attack_phase_step_" + std::to_string(i));
    }

    // Test Case 2: Release Phase (Drop to zero)
    for (int i = 0; i < 250; ++i) {
        float input = 0.0f;
        expected_env = calc_expected(expected_env, input, a_coeff, r_coeff);
        
        float result = rtk::guarded(run, [&] { return k.process(input); });
        rtk::expect(run, rtk::near(result, expected_env, 1e-6), "release_phase_step_" + std::to_string(i));
    }

    // Test Case 3: Reset and state check
    rtk::guarded_void(run, [&] { k.reset(); });
    float level = rtk::guarded(run, [&]{ return k.get_level(); });
    rtk::expect(run, rtk::near(level, 0.0f, 1e-9), "reset_to_zero");

    // Test Case 4: Negative peaks and oscillation
    expected_env = 0.0f;
    float test_inputs[] = { -1.0f, 0.5f, -0.2f, 0.0f, -1.0f };
    for (size_t i = 0; i < 5; ++i) {
        float val = test_inputs[i];
        expected_env = calc_expected(expected_env, val, a_coeff, r_coeff);
        float result = rtk::guarded(run, [&] { return k.process(val); });
        rtk::expect(run, rtk::near(result, expected_env, 1e-6), "oscillation_step_" + std::to_string(i));
    }

    return rtk::report(run);
}
