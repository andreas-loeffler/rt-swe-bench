#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    ExponentialMovingAverage ema;
    const double alpha = 0.1;
    ema.configure(alpha);

    // Case 1: Pop before push
    rtk::expect(run, rtk::near(ema.pop(), 0.0), "Initial pop is zero");

    // Case 2: First push initialization
    rtk::guarded_void(run, [&] { ema.push(10.0); });
    rtk::expect(run, rtk::near(ema.pop(), 10.0), "First push initializes state");

    // Case 3: Sequence of samples
    std::vector<double> inputs = {20.0, 30.0, 40.0, 10.0, 0.0, -10.0, -20.0};
    double expected = 10.0; // starting value from Case 2

    for (auto val : inputs) {
        rtk::guarded_void(run, [&] { ema.push(val); });
        expected = (alpha * val) + (1.0 - alpha) * expected;
        rtk::expect(run, rtk::near(ema.pop(), expected), "EMA calculation correctness");
    }

    // Case 4: Stress test / Convergence
    // Feed 500 samples of a constant value to check convergence
    const double target = 100.0;
    for (int i = 0; i < 500; ++i) {
        rtk::guarded_void(run, [&] { ema.push(target); });
        expected = (alpha * target) + (1.0 - alpha) * expected;
        
        double actual = rtk::guarded(run, [&] { return ema.pop(); });
        rtk::expect(run, rtk::near(actual, expected), "Convergence stress test");
    }

    // Final check: the value should be very close to 100.0 now
    rtk::expect(run, rtk::near(ema.pop(), target, 1e-5), "Converged to target");

    return rtk::report(run);
}
