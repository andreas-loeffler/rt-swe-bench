#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <cmath>

int main() {
    rtk::setup();
    rtk::Run run;

    SignalPeakAccumulator k;
    const size_t batch_size = 100;
    k.configure(batch_size);

    float local_peak = 0.0f;
    size_t local_count = 0;

    // Test 1000 samples (10 full batches)
    for (int i = 0; i < 1000; ++i) {
        // Generate a deterministic signal: sine wave with a spike
        float val = std::sin(i * 0.1f);
        if (i == 50) val = 10.0f; // Spike in batch 1
        if (i == 150) val = -15.0f; // Spike in batch 2

        local_peak = std::max(local_peak, std::abs(val));
        local_count++;

        bool expect_report = (local_count == batch_size);
        float expected_val = expect_report ? local_peak : 0.0f;
        
        if (expect_report) {
            local_peak = 0.0f;
            local_count = 0;
        }

        float actual = rtk::guarded(run, [&] { return k.process(val); });
        rtk::near(run, actual, expected_val, 1e-6f);
    }

    // Test edge case: batch size 1 (every sample reported)
    SignalPeakAccumulator k_small;
    k_small.configure(1);
    for (int i = 0; i < 10; ++i) {
        float val = static_cast<float>(i);
        float actual = rtk::guarded(run, [&] { return k_small.process(val); });
        rtk::near(run, actual, std::abs(val), 1e-6f);
    }

    return rtk::report(run);
}
