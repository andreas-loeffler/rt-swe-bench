#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <algorithm>

int main() {
    rtk::setup();
    rtk::Run run;

    MedianFilter filter;
    const size_t win_size = 5;
    filter.configure(win_size);

    std::vector<float> test_inputs;
    // Generate 600 samples: mixture of trends and spikes
    for (int i = 0; i < 600; ++i) {
        if (i % 10 == 0) test_inputs.push_back(static_cast<float>(i * 100)); // Spike
        else test_inputs.push_back(static_cast<float>(i)); // Linear trend
    }

    // Local shadow state for verification
    std::vector<float> shadow_win;

    for (size_t i = 0; i < test_inputs.size(); ++i) {
        float val = test_inputs[i];

        rtk::guarded_void(run, [&] { filter.push(val); });
        
        shadow_win.push_back(val);
        if (shadow_win.size() > win_size) {
            shadow_win.erase(shadow_win.begin());
        }

        std::vector<float> sorted_shadow = shadow_win;
        std::sort(sorted_shadow.begin(), sorted_shadow.end());
        
        float expected = 0.0f;
        if (!sorted_shadow.empty()) {
            size_t n = sorted_shadow.size();
            if (n % 2 != 0) expected = sorted_shadow[n / 2];
            else expected = (sorted_shadow[n / 2 - 1] + sorted_shadow[n / 2]) * 0.5f;
        }

        float actual = rtk::guarded(run, [&] { return filter.get_median(); });
        rtk::near(run, actual, expected, 1e-6f);
    }

    // Edge case: empty filter check (re-configure)
    filter.configure(3);
    float empty_median = rtk::guarded(run, [&] { return filter.get_median(); });
    rtk::expect(run, empty_median == 0.0f, "Empty filter returns 0");

    return rtk::report(run);
}
