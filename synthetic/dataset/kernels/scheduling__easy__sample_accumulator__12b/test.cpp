#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    const size_t window_size = 10;
    SampleAccumulator k;
    k.configure(window_size);

    std::vector<double> inputs;
    std::vector<double> expected_sums;

    // Generate 600 samples to exercise warm-up, sliding window, and wrap-around
    double current_sum = 0.0;
    std::vector<double> history;
    for (int i = 0; i < 600; ++i) {
        double val = static_cast<double>(i % 10);
        inputs.push_back(val);

        history.push_back(val);
        if (history.size() > window_size) {
            current_sum -= history[history.size() - window_size - 1];
        }
        current_sum += val;
        expected_sums.push_back(current_sum);
    }

    for (size_t i = 0; i < inputs.size(); ++i) {
        double result = rtk::guarded(run, [&] {
            return k.process(inputs[i]);
        });
        rtk::expect(run, result == expected_sums[i], "Sample " + std::to_string(i));
    }

    // Edge case: window size 1
    SampleAccumulator k2;
    k2.configure(1);
    for (int i = 0; i < 50; ++i) {
        double val = static_cast<double>(i);
        double result = rtk::guarded(run, [&] {
            return k2.process(val);
        });
        rtk::expect(run, result == val, "Window size 1, Sample " + std::to_string(i));
    }

    return rtk::report(run);
}
