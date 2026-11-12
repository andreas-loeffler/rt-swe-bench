#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    OnePoleFilter filter;
    float fs = 48000.0f;
    float fc = 1000.0f;
    
    filter.configure(fs, fc);

    // To verify correctness, we calculate alpha independently
    float tau = 1.0f / (2.0f * 3.14159265358979323846f * fc);
    float dt = 1.0f / fs;
    float alpha = dt / (tau + dt);

    // Test Case 1: Step Response (500 samples)
    float expected_z1 = 0.0f;
    for (int i = 0; i < 500; ++i) {
        float input = 1.0f;
        float expected_out = alpha * input + (1.0f - alpha) * expected_z1;
        expected_z1 = expected_out;

        float actual_out = 0.0f;
        rtk::guarded(run, [&] {
            actual_out = filter.process(input);
            return actual_out;
        });
        
        rtk::near(run, actual_out, expected_out, 1e-6f);
    }

    // Test Case 2: Reset
    rtk::guarded_void(run, [&] {
        filter.reset();
    });

    // Test Case 3: Impulse Response (100 samples)
    expected_z1 = 0.0f;
    for (int i = 0; i < 100; ++i) {
        float input = (i == 0) ? 1.0f : 0.0f;
        float expected_out = alpha * input + (1.0f - alpha) * expected_z1;
        expected_z1 = expected_out;

        float actual_out = 0.0f;
        rtk::guarded(run, [&] {
            actual_out = filter.process(input);
            return actual_out;
        });
        
        rtk::near(run, actual_out, expected_out, 1e-6f);
    }

    // Test Case 4: Ramp Response (100 samples)
    filter.reset();
    expected_z1 = 0.0f;
    for (int i = 0; i < 100; ++i) {
        float input = static_cast<float>(i) * 0.01f;
        float expected_out = alpha * input + (1.0f - alpha) * expected_z1;
        expected_z1 = expected_out;

        float actual_out = 0.0f;
        rtk::guarded(run, [&] {
            actual_out = filter.process(input);
            return actual_out;
        });
        
        rtk::near(run, actual_out, expected_out, 1e-6f);
    }

    return rtk::report(run);
}
