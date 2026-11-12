#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <numbers>

int main() {
    rtk::setup();
    rtk::Run run;

    DerivativeFilter filter;
    const double fs = 1000.0;
    const double fc = 10.0;
    filter.configure(fs, fc);

    // Local reference state for validation
    double ref_prev_input = 0.0;
    double ref_prev_output = 0.0;
    double omega = 2.0 * std::numbers::pi * fc;
    double alpha = omega / (fs + omega);

    // Generate a ramp signal: input = i * (1.0/fs)
    // The derivative of a ramp x(t) = kt is k.
    // Here x(n) = n * dt, so derivative is (dt*n - dt(n-1))/dt = 1.0
    for (int i = 0; i < 1000; ++i) {
        double input = i * (1.0 / fs);
        
        double result = rtk::guarded(run, [&] { 
            return filter.process(input); 
        });

        // Compute expected value independently
        double raw_der = (input - ref_prev_input) * fs;
        double expected = alpha * raw_der + (1.0 - alpha) * ref_prev_output;
        
        rtk::expect(run, rtk::near(result, expected), "Step " + std::to_string(i));
        
        ref_prev_input = input;
        ref_prev_output = expected;
    }

    return rtk::report(run);
}
