#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    SlewRateLimiter limiter;
    const double max_delta = 0.1;
    const double start_val = 0.0;
    limiter.configure(max_delta, start_val);

    // Pre-compute expected values to avoid logic duplication in guarded blocks
    std::vector<double> inputs;
    std::vector<double> expected;
    
    double current_expected = start_val;
    
    // Test Case 1: Positive slew (500 steps)
    for (int i = 0; i < 500; ++i) {
        double input = 10.0; // Constant high target
        double diff = input - current_expected;
        if (diff > max_delta) current_expected += max_delta;
        else if (diff < -max_delta) current_expected -= max_delta;
        else current_expected = input;
        
        inputs.push_back(input);
        expected.push_back(current_expected);
    }

    // Test Case 2: Negative slew (500 steps)
    for (int i = 0; i < 500; ++i) {
        double input = -10.0; // Constant low target
        double diff = input - current_expected;
        if (diff > max_delta) current_expected += max_delta;
        else if (diff < -max_delta) current_expected -= max_delta;
        else current_expected = input;
        
        inputs.push_back(input);
        expected.push_back(current_expected);
    }

    // Test Case 3: Small changes (No limiting)
    for (int i = 0; i < 500; ++i) {
        double input = current_expected + (max_delta * 0.5);
        current_expected = input;
        
        inputs.push_back(input);
        expected.push_back(current_expected);
    }

    // Execute real-time calls
    for (size_t i = 0; i < inputs.size(); ++i) {
        double result = rtk::guarded(run, [&, i] { 
            return limiter.process(inputs[i]); 
        });
        rtk::expect(run, rtk::near(result, expected[i]), "Sample " + std::to_string(i));
    }

    return rtk::report(run);
}
