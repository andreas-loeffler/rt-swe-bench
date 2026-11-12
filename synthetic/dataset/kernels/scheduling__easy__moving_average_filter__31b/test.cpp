#include "kernel.hpp" // Test against reference implementation logic
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    MovingAverageFilter filter;
    const size_t N = 10;
    filter.configure(N);

    // Test warm-up and steady state wrap-around
    // Total iterations > 500 to exercise wrap-around multiple times
    std::vector<double> inputs(600);
    for (size_t i = 0; i < inputs.size(); ++i) {
        inputs[i] = static_cast<double>(i);
    }

    double expected_sum = 0.0;
    for (size_t i = 0; i < inputs.size(); ++i) {
        double val = inputs[i];
        
        // Independent calculation of expected value
        expected_sum += val;
        if (i >= N) {
            expected_sum -= inputs[i - N];
        }
        
        size_t current_window_size = (i < N) ? (i + 1) : N;
        double expected_mean = expected_sum / current_window_size;

        double result = rtk::guarded(run, [&]() {
            return filter.process(val);
        });

        rtk::expect(run, rtk::near(result, expected_mean), "Step " + std::to_string(i));
    }

    return rtk::report(run);
}
