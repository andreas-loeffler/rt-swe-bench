#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    MovingAverageFilter filter;
    const size_t N = 10;
    filter.configure(N);

    std::vector<double> inputs;
    // Create 1000 deterministic inputs: a simple ramp
    for (int i = 0; i < 1000; ++i) {
        inputs.push_back(static_cast<double>(i));
    }

    // Local ground truth state
    std::vector<double> history;
    double expected_sum = 0.0;

    for (size_t i = 0; i < inputs.size(); ++i) {
        double val = inputs[i];
        
        // Calculate expected moving average manually
        history.push_back(val);
        expected_sum += val;
        if (history.size() > N) {
            expected_sum -= history[history.size() - N - 1];
        }
        double expected_avg = expected_sum / history.size();
        if (history.size() > N) {
            expected_avg = expected_sum / N;
        }

        double actual_avg = rtk::guarded(run, [&] {
            return filter.process(val);
        });

        std::string label = "Iter " + std::to_string(i);
        rtk::expect(run, rtk::near(actual_avg, expected_avg), label);
    }

    return rtk::report(run);
}
