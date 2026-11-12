#include "kernel.hpp" // Using reference as implementation
#include "rtk/check.hpp"
#include <vector>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    ExponentialMovingAverageFilter filter;
    const double alpha = 0.1;
    
    // configure is called once and may allocate, so it's called outside guarded blocks
    // or inside a guarded block if the harness supports it, but the spec says 
    // "configure(...) is called once before any real-time call."
    filter.configure(alpha);

    double expected = 0.0;
    bool initialized = false;

    // Test 1: Sequence of 500 samples
    for (int i = 0; i < 500; ++i) {
        double val = static_cast<double>(i);
        
        rtk::guarded_void(run, [&] {
            filter.push(val);
        });

        if (!initialized) {
            expected = val;
            initialized = true;
        } else {
            expected = (alpha * val) + (1.0 - alpha) * expected;
        }

        double result = rtk::guarded(run, [&] {
            return filter.get_value();
        });

        rtk::expect(run, rtk::near(result, expected, 1e-9), "EMA Sequence Value " + std::to_string(i));
    }

    // Test 2: Reset
    rtk::guarded_void(run, [&] {
        filter.reset();
    });
    initialized = false;
    expected = 0.0;

    // Test 3: Post-reset behavior
    for (int i = 0; i < 100; ++i) {
        double val = 100.0 + i;
        rtk::guarded_void(run, [&] {
            filter.push(val);
        });

        if (!initialized) {
            expected = val;
            initialized = true;
        } else {
            expected = (alpha * val) + (1.0 - alpha) * expected;
        }

        double result = rtk::guarded(run, [&] {
            return filter.get_value();
        });
        rtk::expect(run, rtk::near(result, expected, 1e-9), "EMA Post-Reset Value " + std::to_string(i));
    }

    return rtk::report(run);
}
