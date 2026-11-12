#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    VelocitySmoother k;
    double alpha = 0.5;
    k.configure(alpha);

    // Test case: Step response
    // At t=0, velocity is 10.0. Since it's the first call, it should return 10.0.
    // Subsequent calls with 10.0 should remain 10.0.
    // Then jump to 20.0 and observe convergence.
    
    double current_val = 10.0;
    {
        double result = rtk::guarded(run, [&] {
            return k.process(current_val);
        });
        rtk::expect(run, result == 10.0, "Initial value should be returned");
    }

    // Test sequence: Jump to 20.0
    // Formula: S_t = alpha * V_t + (1-alpha) * S_{t-1}
    // S_1 = 0.5 * 20.0 + 0.5 * 10.0 = 15.0
    // S_2 = 0.5 * 20.0 + 0.5 * 15.0 = 17.5
    // S_3 = 0.5 * 20.0 + 0.5 * 17.5 = 18.75
    
    std::vector<double> inputs = {20.0, 20.0, 20.0, 20.0, 20.0};
    std::vector<double> expected = {15.0, 17.5, 18.75, 19.375, 19.6875};

    for (size_t i = 0; i < inputs.size(); ++i) {
        double result = rtk::guarded(run, [&] {
            return k.process(inputs[i]);
        });
        rtk::expect(run, rtk::near(result, expected[i], 1e-9), "Smooth step response");
    }

    // Stress test: 500 iterations of random-ish noise
    double steady_input = 100.0;
    double steady_expected = 19.6875; // Current state from previous loop
    
    for (int i = 0; i < 500; ++i) {
        // Predict next
        steady_expected = alpha * steady_input + (1.0 - alpha) * steady_expected;
        
        double result = rtk::guarded(run, [&] {
            return k.process(steady_input);
        });
        rtk::expect(run, rtk::near(result, steady_expected, 1e-9), "Steady state convergence");
    }

    return rtk::report(run);
}
