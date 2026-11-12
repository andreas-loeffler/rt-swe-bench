#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    MovingAverageFilter k;
    const size_t window_size = 10;
    k.configure(window_size);

    // Test 1: Fill the buffer (count < window_size)
    // Expected: Average of first N samples
    std::vector<float> inputs_warmup;
    for (int i = 0; i < 10; ++i) {
        inputs_warmup.push_back(static_cast<float>(i));
    }

    float current_sum = 0.0f;
    for (size_t i = 0; i < inputs_warmup.size(); ++i) {
        float input = inputs_warmup[i];
        current_sum += input;
        float result = rtk::guarded(run, [&] { return k.process(input); });
        rtk::near(result, current_sum / (i + 1), 1e-5f, "Warmup average");
    }

    // Test 2: Steady state (count == window_size)
    // We have 10 samples (0-9). Sum = 45. Next input is 10.
    // Oldest (0) is removed, 10 is added. Sum = 45 - 0 + 10 = 55. Avg = 5.5
    for (int i = 10; i < 510; ++i) {
        float input = static_cast<float>(i);
        
        // Manual calculation for comparison
        static std::vector<float> ref_buffer;
        if (ref_buffer.empty()) {
            for (size_t j = 0; j < 10; ++j) ref_buffer.push_back(static_cast<float>(j));
        }
        
        float oldest = ref_buffer[0];
        ref_buffer.erase(ref_buffer.begin());
        ref_buffer.push_back(input);
        
        float ref_sum = 0.0f;
        for (float v : ref_buffer) ref_sum += v;
        float expected = ref_sum / 10.0f;

        float result = rtk::guarded(run, [&] { return k.process(input); });
        rtk::near(result, expected, 1e-5f, "Steady state average");
    }

    return rtk::report(run);
}
