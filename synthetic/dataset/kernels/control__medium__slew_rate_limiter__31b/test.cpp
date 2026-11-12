#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    SlewLimiter limiter;
    const double max_step = 0.1;
    const double start_val = 0.0;
    limiter.configure(max_step, start_val);

    double expected = start_val;

    // Test Case 1: Gradual climb (Under the limit)
    for (int i = 0; i < 200; ++i) {
        double target = 0.05 * (i + 1); // Slow increase
        double actual = rtk::guarded(run, [&] { return limiter.process(target); });
        
        // Compute expected locally: simple lerp/clamp
        double diff = target - expected;
        if (diff > max_step) expected += max_step;
        else if (diff < -max_step) expected -= max_step;
        else expected = target;

        rtk::expect(run, rtk::near(actual, expected), "gradual_climb_" + std::to_string(i));
    }

    // Test Case 2: Hard Step Up (Saturation)
    // Target is far away, should take many steps to reach
    double hard_target = 10.0;
    for (int i = 0; i < 200; ++i) {
        double actual = rtk::guarded(run, [&] { return limiter.process(hard_target); });
        
        double diff = hard_target - expected;
        if (diff > max_step) expected += max_step;
        else if (diff < -max_step) expected -= max_step;
        else expected = hard_target;

        rtk::expect(run, rtk::near(actual, expected), "hard_step_up_" + std::to_string(i));
    }

    // Test Case 3: Hard Step Down (Saturation)
    hard_target = -10.0;
    for (int i = 0; i < 200; ++i) {
        double actual = rtk::guarded(run, [&] { return limiter.process(hard_target); });
        
        double diff = hard_target - expected;
        if (diff > max_step) expected += max_step;
        else if (diff < -max_step) expected -= max_step;
        else expected = hard_target;

        rtk::expect(run, rtk::near(actual, expected), "hard_step_down_" + std::to_string(i));
    }

    // Test Case 4: Jitter/Oscillation
    for (int i = 0; i < 100; ++i) {
        double target = (i % 2 == 0) ? 1.0 : -1.0;
        double actual = rtk::guarded(run, [&] { return limiter.process(target); });
        
        double diff = target - expected;
        if (diff > max_step) expected += max_step;
        else if (diff < -max_step) expected -= max_step;
        else expected = target;

        rtk::expect(run, rtk::near(actual, expected), "jitter_" + std::to_string(i));
        
        double current = rtk::guarded(run, [&] { return limiter.get_current(); });
        rtk::expect(run, rtk::near(current, expected), "get_current_jitter_" + std::to_string(i));
    }

    return rtk::report(run);
}
