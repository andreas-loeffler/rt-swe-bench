#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    LinearGainRamp k;
    k.configure(1024);

    // Test 0: Empty block
    rtk::guarded_void(run, [&] {
        k.process(nullptr, nullptr, 0, 1.0f, 0.5f);
    });

    // Test 1: Single sample
    {
        float in[1] = { 10.0f };
        float out[1] = { 0.0f };
        rtk::guarded_void(run, [&] {
            k.process(in, out, 1, 0.5f, 1.0f);
        });
        rtk::expect(run, out[0] == 5.0f, "Single sample gain");
    }

    // Test 2: Linear ramp 500 iterations
    // We use a loop to create > 500 calls as requested
    for (int i = 0; i < 500; ++i) {
        size_t n = 2;
        float input[2] = { 1.0f, 1.0f };
        float output[2] = { 0.0f, 0.0f };
        
        float start = static_cast<float>(i) / 500.0f;
        float end = 1.0f - start;

        rtk::guarded_void(run, [&] {
            k.process(input, output, n, start, end);
        });

        // Expected: out[0] = 1.0 * start, out[1] = 1.0 * end
        rtk::expect(run, rtk::near(output[0], start), "Ramp start");
        rtk::expect(run, rtk::near(output[1], end), "Ramp end");
    }

    // Test 3: High values and negative gains
    {
        float input[3] = { 1.0f, 2.0f, 3.0f };
        float output[3] = { 0.0f, 0.0f, 0.0f };
        // start -1.0, end 1.0. steps: -1.0, 0.0, 1.0
        rtk::guarded_void(run, [&] {
            k.process(input, output, 3, -1.0f, 1.0f);
        });
        rtk::expect(run, rtk::near(output[0], -1.0f), "Negative gain");
        rtk::expect(run, rtk::near(output[1], 0.0f), "Zero gain");
        rtk::expect(run, rtk::near(output[2], 3.0f), "Positive gain");
    }

    return rtk::report(run);
}
