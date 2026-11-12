#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    ExponentialSmoothingFilter filter;
    
    // Test Case 1: Alpha = 1.0 (Bypass)
    filter.configure(1.0f);
    for (int i = 0; i < 250; ++i) {
        float input = static_cast<float>(i) * 0.1f;
        float expected = input;
        float actual = rtk::guarded(run, [&] { return filter.process(input); });
        rtk::expect(run, rtk::near(actual, expected), "Bypass mode: " + std::to_string(i));
    }

    // Test Case 2: Alpha = 0.0 (Hold/Freeze)
    filter.configure(0.0f);
    // Initialize state to 0 via configure, then push values
    for (int i = 0; i < 250; ++i) {
        float input = 100.0f;
        float expected = 0.0f;
        float actual = rtk::guarded(run, [&] { return filter.process(input); });
        rtk::expect(run, rtk::near(actual, expected), "Freeze mode: " + std::to_string(i));
    }

    return rtk::report(run);
}
