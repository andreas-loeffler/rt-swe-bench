#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    CascadedBiquadIIR filter;
    const size_t num_sec = 3;
    const float fs = 44100.0f;

    filter.configure(num_sec, fs);

    // Test 1: Complex impulse response to verify every coefficient path
    // We set specific coefficients that exercise b0, b1, b2, a1, a2
    // Section 0: Simple gain (b0=0.5)
    // Section 1: a1 and b1 influence (b0=1, b1=0.1, a1=0.2)
    // Section 2: a2 and b2 influence (b0=1, b2=0.1, a2=0.3)
    rtk::guarded_void(run, [&] {
        filter.set_coefficients(0, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f);
        filter.set_coefficients(1, 1.0f, 0.1f, 0.0f, 0.2f, 0.0f);
        filter.set_coefficients(2, 1.0f, 0.0f, 0.1f, 0.0f, 0.3f);
    });

    // Reference implementation of Direct Form II Transposed
    auto ref_process = [&](float x, std::vector<float>& s1, std::vector<float>& s2, 
                           const std::vector<std::vector<float>>& coeffs) {
        float out = x;
        for (size_t i = 0; i < coeffs.size(); ++i) {
            float b0 = coeffs[i][0];
            float b1 = coeffs[i][1];
            float b2 = coeffs[i][2];
            float a1 = coeffs[i][3];
            float a2 = coeffs[i][4];

            float y = b0 * out + s1[i];
            s1[i] = b1 * out - a1 * y + s2[i];
            s2[i] = b2 * out - a2 * y;
            out = y;
        }
        return out;
    };

    std::vector<std::vector<float>> ref_coeffs = {
        {0.5f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 0.1f, 0.0f, 0.2f, 0.0f},
        {1.0f, 0.0f, 0.1f, 0.0f, 0.3f}
    };
    std::vector<float> s1_ref(num_sec, 0.0f);
    std::vector<float> s2_ref(num_sec, 0.0f);

    // Process a sequence of inputs to ensure state accumulation is correct
    std::vector<float> inputs = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    for (size_t i = 0; i < inputs.size(); ++i) {
        float out = 0;
        rtk::guarded(run, [&] {
            out = filter.process(inputs[i]);
            return out;
        });
        float expected = ref_process(inputs[i], s1_ref, s2_ref, ref_coeffs);
        rtk::expect(run, rtk::near(out, expected), "Sample " + std::to_string(i));
    }

    // Test stability boundary conditions
    // Case A: Stable
    rtk::guarded_void(run, [&] {
        filter.set_coefficients(0, 1.0f, 0.0f, 0.0f, 0.5f, 0.1f); 
    });
    rtk::expect(run, rtk::guarded(run, [&] { return filter.is_stable(); }), "Stable case");

    // Case B: |a2| >= 1 (Unstable)
    rtk::guarded_void(run, [&] {
        filter.set_coefficients(0, 1.0f, 0.0f, 0.0f, 0.0f, 1.1f); 
    });
    rtk::expect(run, !rtk::guarded(run, [&] { return filter.is_stable(); }), "Unstable a2 > 1");

    // Case C: |a1| >= 1 + a2 (Unstable)
    rtk::guarded_void(run, [&] {
        filter.set_coefficients(0, 1.0f, 0.0f, 0.0f, 2.0f, 0.5f); // 2.0 >= 1.5
    });
    rtk::expect(run, !rtk::guarded(run, [&] { return filter.is_stable(); }), "Unstable a1 > 1+a2");

    // Test reset
    rtk::guarded_void(run, [&] {
        filter.set_coefficients(0, 1.0f, 0.1f, 0.1f, 0.1f, 0.1f);
        filter.process(1.0f); // load state
        filter.reset();
    });

    float reset_out = 0;
    rtk::guarded(run, [&] {
        reset_out = filter.process(1.0f);
        return reset_out;
    });
    // After reset and processing 1.0, output should be exactly b0*1.0 = 1.0 (for the first section)
    // But it goes through 3 sections. Let's use identity for others.
    rtk::guarded_void(run, [&] {
        filter.reset();
        filter.set_coefficients(0, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        filter.set_coefficients(1, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        filter.set_coefficients(2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        filter.process(1.0f); // load state
        filter.reset();
    });

    float final_out = 0;
    rtk::guarded(run, [&] {
        final_out = filter.process(1.0f);
        return final_out;
    });
    rtk::expect(run, rtk::near(final_out, 1.0f), "Reset check");

    return rtk::report(run);
}
