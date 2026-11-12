#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;

    const size_t window_size = 10;
    MovingAverage k;
    k.configure(window_size);

    std::vector<double> inputs;
    std::vector<double> expected_outputs;
    
    // Generate 500 deterministic inputs
    // Sequence: 0, 1, 2, ..., 499
    // The mean should transition from growing average to sliding window average
    for (int i = 0; i < 500; ++i) {
        inputs.push_back(static_cast<double>(i));
        
        // Compute expected independently
        static std::vector<double> history;
        history.push_back(static_cast<double>(i));
        if (history.size() > window_size) {
            history.erase(history.begin());
        }
        double sum = std::accumulate(history.begin(), history.end(), 0.0);
        expected_outputs.push_back(sum / history.size());
    }

    for (size_t i = 0; i < inputs.size(); ++i) {
        double result = rtk::guarded(run, [&] { 
            return k.process(inputs[i]); 
        });
        rtk::near(result, expected_outputs[i], 1e-9, "Match at index " + std::to_string(i));
    }

    return rtk::report(run);
}
