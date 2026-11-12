#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    ExponentialMovingAverage ema;
    const double alpha = 0.1;
    ema.configure(alpha);

    // Test Case 1: Convergence from initialization
    // This catches changes to the logic of alpha application or the state update.
    double expected = 0.0;
    bool initialized = false;
    
    for (int i = 0; i < 600; ++i) {
        double input = 10.0; 
        
        double actual = rtk::guarded(run, [&] { 
            return ema.process(input); 
        });

        // Reference logic
        if (!initialized) {
            expected = input;
            initialized = true;
        } else {
            expected = (alpha * input) + (1.0 - alpha) * expected;
        }

        rtk::expect(run, rtk::near(actual, expected), "Convergence step " + std::to_string(i));
    }

    // Test Case 2: State query
    double state_val = rtk::guarded(run, [&] { 
        return ema.get_state(); 
    });
    rtk::expect(run, rtk::near(state_val, expected), "State query match");

    // Test Case 3: Reset and re-init
    // This catches changes to m_initialized = false or state initialization to 0.0
    rtk::guarded_void(run, [&] { ema.reset(); });
    
    // After reset, the first sample MUST be the input value regardless of alpha
    double input_reset = 5.0;
    double actual_reset = rtk::guarded(run, [&] { 
        return ema.process(input_reset); 
    });
    rtk::expect(run, rtk::near(actual_reset, input_reset), "Reset first-sample init");

    // Test Case 4: High alpha (no smoothing)
    // This catches if m_alpha is ignored or applied incorrectly
    ExponentialMovingAverage ema_fast;
    ema_fast.configure(1.0); 
    for (int i = 0; i < 100; ++i) {
        double val = static_cast<double>(i * 7);
        double res = rtk::guarded(run, [&] { return ema_fast.process(val); });
        rtk::expect(run, rtk::near(res, val), "Alpha 1.0 follow " + std::to_string(i));
    }

    // Test Case 5: Low alpha (full smoothing / hold)
    // This catches if the filter updates when it shouldn't (alpha = 0.0)
    ExponentialMovingAverage ema_slow;
    ema_slow.configure(0.0);
    double first_val = 10.0;
    rtk::guarded(run, [&] { return ema_slow.process(first_val); });
    for (int i = 0; i < 100; ++i) {
        double val = 20.0;
        double res = rtk::guarded(run, [&] { return ema_slow.process(val); });
        rtk::expect(run, rtk::near(res, first_val), "Alpha 0.0 hold " + std::to_string(i));
    }

    return rtk::report(run);
}
