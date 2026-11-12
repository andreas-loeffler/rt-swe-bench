#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    DecimationFilter filter;
    const size_t N = 4;
    filter.configure(N);

    // Generate 1000 samples of a simple ramp signal
    std::vector<float> inputs;
    for (int i = 0; i < 1000; ++i) {
        inputs.push_back(static_cast<float>(i));
    }

    float current_sum = 0.0f;
    size_t current_count = 0;

    for (size_t i = 0; i < inputs.size(); ++i) {
        float sample = inputs[i];
        float output = 0.0f;
        
        // Expected calculation performed in plain code
        current_sum += sample;
        current_count++;
        bool expect_out = false;
        float expect_val = 0.0f;
        if (current_count == N) {
            expect_out = true;
            expect_val = current_sum / static_cast<float>(N);
            current_sum = 0.0f;
            current_count = 0;
        }

        bool actual_out = rtk::guarded(run, [&]() {
            return filter.push(sample, output);
        });

        rtk::expect(run, actual_out == expect_out, "push_trigger_state_" + std::to_string(i));
        if (actual_out) {
            rtk::expect(run, rtk::near(output, expect_val), "push_value_" + std::to_string(i));
        }
    }

    return rtk::report(run);
}
