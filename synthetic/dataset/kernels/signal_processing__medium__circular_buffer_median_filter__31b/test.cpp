#include "kernel.hpp" // Using reference as the implementation for the benchmark
#include "rtk/check.hpp"
#include <vector>
#include <algorithm>

float calculate_expected_median(std::vector<float> window) {
    if (window.empty()) return 0.0f;
    std::sort(window.begin(), window.end());
    size_t n = window.size();
    if (n % 2 == 1) return window[n / 2];
    return (window[n / 2 - 1] + window[n / 2]) * 0.5f;
}

int main() {
    rtk::setup();
    rtk::Run run;
    MedianFilter k;
    
    const size_t window_size = 5;
    k.configure(window_size);

    std::vector<float> test_inputs;
    // Generate 600 deterministic inputs: ramp, spikes, and noise
    for (int i = 0; i < 200; ++i) test_inputs.push_back(static_cast<float>(i)); 
    for (int i = 0; i < 200; ++i) test_inputs.push_back(i % 2 == 0 ? 1000.0f : -1000.0f);
    for (int i = 0; i < 200; ++i) test_inputs.push_back(static_cast<float>(i * 0.5));

    std::vector<float> current_window;
    for (float val : test_inputs) {
        rtk::guarded_void(run, [&] { 
            k.push(val); 
        });
        
        current_window.push_back(val);
        if (current_window.size() > window_size) {
            current_window.erase(current_window.begin());
        }

        float expected = calculate_expected_median(current_window);
        float actual = rtk::guarded(run, [&] { 
            return k.process(); 
        });

        rtk::near(run, actual, expected, 1e-6f);
    }

    return rtk::report(run);
}
