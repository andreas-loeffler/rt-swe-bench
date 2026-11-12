#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    ExponentialSmoother smoother;
    
    // Test Case 1: Standard smoothing
    double alpha = 0.1;
    smoother.configure(alpha);

    double current_expected = 0.0;
    bool first = true;

    for (int i = 0; i < 500; ++i) {
        double input = std::sin(i * 0.1);
        
        double actual = rtk::guarded(run, [&] { 
            return smoother.process(input); 
        });

        if (first) {
            current_expected = input;
            first = false;
        } else {
            current_expected = alpha * input + (1.0 - alpha) * current_expected;
        }

        rtk::expect(run, rtk::near(actual, current_expected), "Smooth value match at step " + std::to_string(i));
    }

    // Test Case 2: Alpha = 1.0 (No smoothing)
    smoother.configure(1.0);
    for (int i = 0; i < 100; ++i) {
        double input = static_cast<double>(i * 123 % 100);
        double actual = rtk::guarded(run, [&] { 
            return smoother.process(input); 
        });
        rtk::expect(run, rtk::near(actual, input), "Identity match alpha=1.0");
    }

    // Test Case 3: Alpha = 0.0 (Frozen state)
    smoother.configure(0.0);
    double first_val = 42.0;
    rtk::guarded_void(run, [&] { smoother.process(first_val); });
    for (int i = 0; i < 100; ++i) {
        double actual = rtk::guarded(run, [&] { 
            return smoother.process(999.0); 
        });
        rtk::expect(run, rtk::near(actual, first_val), "Frozen match alpha=0.0");
    }

    return rtk::report(run);
}
