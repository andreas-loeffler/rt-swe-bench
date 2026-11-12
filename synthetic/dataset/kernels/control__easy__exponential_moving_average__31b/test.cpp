#include "kernel.hpp" // Testing the reference implementation
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    ExponentialMovingAverage ema;
    const double alpha = 0.1;
    ema.configure(alpha);

    // Generate a deterministic signal: a step function followed by a ramp
    std::vector<double> inputs(1000);
    for (size_t i = 0; i < 1000; ++i) {
        if (i < 500) inputs[i] = 10.0;
        else inputs[i] = static_cast<double>(i) * 0.1;
    }

    double expected_state = 0.0;
    bool first = true;

    for (int i = 0; i < 1000; ++i) {
        double input = inputs[i];
        
        // Calculate expected value independently
        if (first) {
            expected_state = input;
            first = false;
        } else {
            expected_state = (alpha * input) + (1.0 - alpha) * expected_state;
        }

        double result = rtk::guarded(run, [&]() {
            return ema.process(input);
        });

        rtk::expect(run, rtk::near(result, expected_state, 1e-9), "EMA value at index " + std::to_string(i));
    }

    return rtk::report(run);
}
