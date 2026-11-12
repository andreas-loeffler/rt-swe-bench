#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    LinearHistogramCounter k;
    const int32_t min_val = 10;
    const int32_t max_val = 20;
    k.configure(min_val, max_val);

    // Use a local vector to keep track of expected values outside the guarded calls
    std::vector<uint64_t> expected(max_val - min_val + 1, 0);

    auto update_expected = [&](int32_t val) {
        if (val >= min_val && val <= max_val) {
            expected[val - min_val]++;
        }
    };

    // Exercise 1: Basic sequence and range boundaries
    for (int i = 0; i < 600; ++i) {
        int32_t val;
        if (i % 10 == 0) val = min_val;           // Lower bound
        else if (i % 10 == 1) val = max_val;      // Upper bound
        else if (i % 10 == 2) val = min_val - 1;  // Just below
        else if (i % 10 == 3) val = max_val + 1;  // Just above
        else val = min_val + (i % (max_val - min_val + 1)); // Interior

        update_expected(val);
        rtk::guarded_void(run, [&] { k.increment(val); });
    }

    // Verify all bins
    for (int32_t v = min_val; v <= max_val; ++v) {
        uint64_t actual = rtk::guarded(run, [&] { return k.get_count(v); });
        rtk::expect(run, actual == expected[v - min_val], "Bin " + std::to_string(v));
    }

    // Verify out of bounds
    rtk::expect(run, rtk::guarded(run, [&] { return k.get_count(min_val - 1); }) == 0, "Underflow");
    rtk::expect(run, rtk::guarded(run, [&] { return k.get_count(max_val + 1); }) == 0, "Overflow");

    // Exercise 2: Reset
    rtk::guarded_void(run, [&] { k.reset(); });
    std::fill(expected.begin(), expected.end(), 0);

    for (int32_t v = min_val; v <= max_val; ++v) {
        rtk::expect(run, rtk::guarded(run, [&] { return k.get_count(v); }) == 0, "Reset check " + std::to_string(v));
    }

    return rtk::report(run);
}
