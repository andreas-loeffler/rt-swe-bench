#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    ExponentialMovingAverage ema;
    const double fs = 1000.0;
    const double tau = 0.1;
    ema.configure(fs, tau);

    // Test 1: Impulse response and convergence
    // Expected: y[n] = alpha * (1 - (1-alpha)^n) for input = 1.0
    double alpha = 1.0 - std::exp(-1.0 / (fs * tau));
    double expected_val = 0.0;
    
    for (int i = 0; i < 600; ++i) {
        double input = 1.0;
        expected_val = alpha * input + (1.0 - alpha) * expected_val;
        
        double result = rtk::guarded(run, [&]() { 
            return ema.step(input); 
        });
        
        rtk::expect(run, rtk::near(result, expected_val), "Convergence Step " + std::to_string(i));
    }

    // Test 2: Steady state persistence
    double steady_state = rtk::guarded(run, [&]() {
        return ema.get_current_value();
    });
    rtk::expect(run, rtk::near(steady_state, expected_val), "Steady State Value");

    // Test 3: Reset behavior
    rtk::guarded_void(run, [&]() { 
        ema.reset(); 
    });
    double after_reset = rtk::guarded(run, [&]() { 
        return ema.get_current_value(); 
    });
    rtk::expect(run, rtk::near(after_reset, 0.0), "Post-Reset Zero");

    // Test 4: Response to negative step
    expected_val = 0.0; // reset happened
    for (int i = 0; i < 100; ++i) {
        double input = -1.0;
        expected_val = alpha * input + (1.0 - alpha) * expected_val;
        
        double result = rtk::guarded(run, [&]() { 
            return ema.step(input); 
        });
        
        rtk::expect(run, rtk::near(result, expected_val), "Negative Step " + std::to_string(i));
    }

    return rtk::report(run);
}
