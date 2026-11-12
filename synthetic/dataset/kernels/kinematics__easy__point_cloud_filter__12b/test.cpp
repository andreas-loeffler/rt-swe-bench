#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <random>
#include <algorithm>

int main() {
    rtk::setup();
    rtk::Run run;

    const size_t max_points = 100;
    const float threshold = 10.0f;
    PointFilter filter;
    filter.configure(max_points, threshold);

    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dist(-20.0f, 20.0f);

    for (int i = 0; i < 500; ++i) {
        // Deterministic logic for test generation
        size_t count = (i % 10) + 1; // 1 to 10 points
        std::vector<float> inputs(count * 3);
        for (size_t j = 0; j < count * 3; ++j) {
            inputs[j] = dist(gen);
        }

        // Expected behavior calculation
        size_t expected_count = 0;
        size_t limit = std::min(count, max_points);
        for (size_t j = 0; j < limit; ++j) {
            float x = inputs[j * 3];
            float y = inputs[j * 3 + 1];
            float z = inputs[j * 3 + 2];
            if ((x * x + y * y + z * z) <= (threshold * threshold)) {
                expected_count++;
            }
        }

        std::vector<float> outputs(count * 3);
        size_t result = rtk::guarded(run, [&] {
            return filter.process(inputs.data(), count, outputs.data());
        });

        rtk::expect(run, result == expected_count, "Count match");
        
        // Verify contents of output
        size_t current_idx = 0;
        for (size_t j = 0; j < limit; ++j) {
            float x = inputs[j * 3];
            float y = inputs[j * 3 + 1];
            float z = inputs[j * 3 + 2];
            if ((x * x + y * y + z * z) <= (threshold * threshold)) {
                rtk::near(outputs[current_idx * 3], x, 1e-5, "x");
                rtk::near(outputs[current_idx * 3 + 1], y, 1e-5, "y");
                rtk::near(outputs[current_idx * 3 + 2], z, 1e-5, "z");
                current_idx++;
            }
        }
    }

    // Edge case: Empty input
    std::vector<float> empty_outputs(10);
    size_t result_empty = rtk::guarded(run, [&] {
        return filter.process(nullptr, 0, empty_outputs.data());
    });
    rtk::expect(run, result_empty == 0, "Empty input");

    // Edge case: Overflowing max_points
    size_t large_count = max_points + 50;
    std::vector<float> large_inputs(large_count * 3, 0.0f);
    std::vector<float> large_outputs(large_count * 3);
    size_t result_large = rtk::guarded(run, [&] {
        return filter.process(large_inputs.data(), large_count, large_outputs.data());
    });
    rtk::expect(run, result_large <= max_points, "Max points limit");

    return rtk::report(run);
}
