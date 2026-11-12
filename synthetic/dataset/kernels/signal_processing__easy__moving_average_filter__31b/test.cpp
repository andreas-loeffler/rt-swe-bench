#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;

    MovingAverageFilter k;
    const size_t window_size = 10;
    k.configure(window_size);

    // Reference state tracking for test validation
    std::vector<double> history;
    
    // 1000 samples to exercise filling, wrap-around and steady state
    for (int i = 0; i < 1000; ++i) {
        double input = static_cast<double>(i);
        
        double result = rtk::guarded(run, [&] { 
            return k.process(input); 
        });

        // Independent calculation of expected value
        history.push_back(input);
        double expected_sum = 0.0;
        size_t count = 0;
        for (auto it = history.rbegin(); it != history.rend() && count < window_size; ++it, ++count) {
            expected_sum += *it;
        }
        double expected_avg = expected_sum / count;

        rtk::expect(run, rtk::near(result, expected_avg), "Average at step " + std::to_string(i));
    }

    return rtk::report(run);
}
