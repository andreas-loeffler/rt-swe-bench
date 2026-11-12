#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    ExponentialMovingAverage ema;
    const float alpha = 0.1f;
    ema.configure(alpha);

    // Test case 1: Initial value
    float input0 = 10.0f;
    float result0 = rtk::guarded(run, [&] { return ema.process(input0); });
    rtk::expect(run, rtk::near(result0, 10.0f), "Initial value should be input");

    // Test case 2: Sequence of 500 values
    float current_expected = 10.0f;
    for (int i = 1; i <= 500; ++i) {
        float input = static_cast<float>(i % 100);
        
        // Independent computation of expected value
        current_expected = (alpha * input) + (1.0f - alpha) * current_expected;
        
        float result = rtk::guarded(run, [&] { return ema.process(input); });
        rtk::expect(run, rtk::near(result, current_expected), "EMA value at step " + std::to_string(i));
    }

    // Edge case: Alpha = 1.0 (No smoothing)
    ExponentialMovingAverage ema_fast;
    ema_fast.configure(1.0f);
    rtk::guarded_void(run, [&] { ema_fast.process(5.0f); }); // init
    float result_fast = rtk::guarded(run, [&] { return ema_fast.process(12.3f); });
    rtk::expect(run, rtk::near(result_fast, 12.3f), "Alpha 1.0 should pass input");

    // Edge case: Alpha = 0.0 (Hold value)
    ExponentialMovingAverage ema_slow;
    ema_slow.configure(0.0f);
    rtk::guarded_void(run, [&] { ema_slow.process(5.0f); }); // init
    float result_slow = rtk::guarded(run, [&] { return ema_slow.process(12.3f); });
    rtk::expect(run, rtk::near(result_slow, 5.0f), "Alpha 0.0 should hold initial value");

    return rtk::report(run);
}
