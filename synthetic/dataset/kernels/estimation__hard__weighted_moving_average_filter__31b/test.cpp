#include "kernel.hpp" // This serves as the kernel implementation
#include "rtk/check.hpp"
#include <vector>
#include <cmath>
#include <string>

float calculate_expected(const std::vector<float>& samples, size_t window_size, WeightMode mode, float sigma) {
    if (samples.empty()) return 0.0f;
    
    size_t n = std::min(samples.size(), window_size);
    float weighted_sum = 0.0f;
    float weight_total = 0.0f;

    for (size_t i = 0; i < n; ++i) {
        float val = samples[samples.size() - 1 - i];
        float w = 0.0f;
        if (mode == WeightMode::Linear) {
            w = static_cast<float>(window_size - i);
        } else {
            float x = static_cast<float>(i);
            w = std::exp(-(x * x) / (2.0f * sigma * sigma));
        }
        weighted_sum += val * w;
        weight_total += w;
    }
    return weighted_sum / weight_total;
}

int main() {
    rtk::setup();
    rtk::Run run;

    WeightedMovingAverage filter;
    const size_t window = 10;
    const float sigma = 1.5f;
    
    // Test 1: Linear Mode
    filter.configure(window, WeightMode::Linear);

    std::vector<float> history;
    for (int i = 0; i < 600; ++i) {
        float val = static_cast<float>(i) * 0.1f;
        
        rtk::guarded_void(run, [&] { filter.push(val); });
        history.push_back(val);
        if (history.size() > window) history.erase(history.begin());

        float result = rtk::guarded(run, [&] { return filter.process(); });
        float expected = calculate_expected(history, window, WeightMode::Linear, sigma);
        rtk::expect(run, rtk::near(result, expected), "Linear step " + std::to_string(i));
    }

    rtk::guarded_void(run, [&] { filter.reset(); });
    rtk::expect(run, rtk::near(rtk::guarded(run, [&] { return filter.process(); }), 0.0f, 1e-6f), "Reset test");

    // Test 2: Gaussian Mode
    filter.configure(window, WeightMode::Gaussian, sigma);

    history.clear();
    for (int i = 0; i < 600; ++i) {
        float val = std::sin(i * 0.1f);
        
        rtk::guarded_void(run, [&] { filter.push(val); });
        history.push_back(val);
        if (history.size() > window) history.erase(history.begin());

        float result = rtk::guarded(run, [&] { return filter.process(); });
        float expected = calculate_expected(history, window, WeightMode::Gaussian, sigma);
        rtk::expect(run, rtk::near(result, expected), "Gaussian step " + std::to_string(i));
    }

    return rtk::report(run);
}
