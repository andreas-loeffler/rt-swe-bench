#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    IirFilter filter;
    const float alpha = 0.1f;
    filter.configure(alpha);

    // Use a simple recursive calculation for expected values
    float expected_prev = 0.0f;
    std::vector<float> inputs;
    std::vector<float> expected_outputs;

    // Generate 501 inputs (0 to 500)
    for (int i = 0; i <= 500; ++i) {
        float input = static_cast<float>(i);
        inputs.push_back(input);
        
        float expected = alpha * input + (1.0f - alpha) * expected_prev;
        expected_outputs.push_back(expected);
        expected_prev = expected;
    }

    // Test sequence
    for (size_t i = 0; i < inputs.size(); ++i) {
        float result = rtk::guarded(run, [&] {
            return filter.process(inputs[i]);
        });
        rtk::near(result, expected_outputs[i], 1e-6f, "filter step " + std::to_string(i));
    }

    return rtk::report(run);
}
