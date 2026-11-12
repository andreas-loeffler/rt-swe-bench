#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <algorithm>

int main() {
    rtk::setup();
    rtk::Run run;

    MedianFilter filter;
    const size_t window_size = 5;
    filter.configure(window_size);

    // Test data: a mix of steady signals, impulses, and ramp
    std::vector<float> inputs;
    for (int i = 0; i < 600; ++i) {
        if (i % 20 == 0) inputs.push_back(100.0f); // Impulse
        else if (i % 20 == 1) inputs.push_back(-100.0f); // Inverse impulse
        else inputs.push_back(static_cast<float>(i) * 0.1f);
    }

    // Local mirror for ground truth calculation
    std::vector<float> window;

    for (float val : inputs) {
        float expected = 0.0f;
        
        // Ground truth calculation (non-RT)
        window.push_back(val);
        if (window.size() > window_size) {
            window.erase(window.begin());
        }
        std::vector<float> sorted_win = window;
        std::sort(sorted_win.begin(), sorted_win.end());
        expected = sorted_win[sorted_win.size() / 2];

        float actual = rtk::guarded(run, [&] { 
            return filter.push(val); 
        });

        rtk::expect(run, rtk::near(actual, expected), "median_match");
    }

    return rtk::report(run);
}
