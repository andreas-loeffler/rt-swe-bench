#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    ExponentialSmoother smoother;
    const double alpha = 0.2;
    const double start_val = 10.0;
    smoother.configure(alpha, start_val);

    // We track the expected value externally using the mathematical definition
    double expected = start_val;
    
    // Test 1: Steady state
    for (int i = 0; i < 100; ++i) {
        double input = 10.0;
        double result = rtk::guarded(run, [&] { return smoother.push(input); });
        
        expected = (alpha * input) + (1.0 - alpha) * expected;
        rtk::expect(run, rtk::near(result, expected), "Steady state check");
    }

    // Test 2: Ramp input
    for (int i = 0; i < 200; ++i) {
        double input = static_cast<double>(i);
        double result = rtk::guarded(run, [&] { return smoother.push(input); });
        
        expected = (alpha * input) + (1.0 - alpha) * expected;
        rtk::expect(run, rtk::near(result, expected), "Ramp input check");
    }

    // Test 3: High frequency jitter (Square wave)
    for (int i = 0; i < 200; ++i) {
        double input = (i % 2 == 0) ? 100.0 : 0.0;
        
        // Check get_current before pushing
        double current = rtk::guarded(run, [&] { return smoother.get_current(); });
        rtk::expect(run, rtk::near(current, expected), "Current value check");
        
        double result = rtk::guarded(run, [&] { return smoother.push(input); });
        expected = (alpha * input) + (1.0 - alpha) * expected;
        rtk::expect(run, rtk::near(result, expected), "Jitter input check");
    }

    return rtk::report(run);
}
