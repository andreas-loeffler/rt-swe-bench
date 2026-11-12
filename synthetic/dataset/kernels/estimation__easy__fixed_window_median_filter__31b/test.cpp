#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <algorithm>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    FixedWindowMedianFilter filter;
    const size_t win_size = 5;
    filter.configure(win_size);

    std::vector<float> inputs;
    // Generate 600 samples: ramp, noise spikes, and steady state
    for (int i = 0; i < 600; ++i) {
        if (i % 10 == 0) {
            inputs.push_back(1000.0f); // Spike
        } else {
            inputs.push_back(static_cast<float>(i));
        }
    }

    std::vector<float> window;
    for (float val : inputs) {
        // Local reference calculation
        window.push_back(val);
        if (window.size() > win_size) {
            window.erase(window.begin());
        }
        
        std::vector<float> sorted_win = window;
        std::sort(sorted_win.begin(), sorted_win.end());
        
        float expected;
        size_t n = sorted_win.size();
        if (n % 2 != 0) {
            expected = sorted_win[n / 2];
        } else {
            expected = (sorted_win[n / 2 - 1] + sorted_win[n / 2]) * 0.5f;
        }

        float result = rtk::guarded(run, [&] { 
            return filter.process(val); 
        });

        rtk::near(run, result, expected, 1e-6f);
    }

    return rtk::report(run);
}
