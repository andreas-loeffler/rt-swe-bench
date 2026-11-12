#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <algorithm>

int main() {
    rtk::setup();
    rtk::Run run;

    SortedWindowMedian k;
    const size_t WINDOW_SIZE = 10;
    k.configure(WINDOW_SIZE);

    // Independent reference logic for validation
    std::vector<double> ref_win;
    auto get_ref_median = [&]() {
        if (ref_win.empty()) return 0.0;
        std::vector<double> sorted = ref_win;
        std::sort(sorted.begin(), sorted.end());
        size_t n = sorted.size();
        if (n % 2 == 1) return sorted[n / 2];
        return (sorted[n / 2 - 1] + sorted[n / 2]) * 0.5;
    };

    // Test filling, wrap-around, and stability
    for (int i = 0; i < 600; ++i) {
        double val = static_cast<double>(i % 100);
        
        // Update reference window
        ref_win.push_back(val);
        if (ref_win.size() > WINDOW_SIZE) {
            ref_win.erase(ref_win.begin());
        }
        double expected = get_ref_median();

        double actual = rtk::guarded(run, [&]() {
            return k.push(val);
        });

        rtk::near(run, actual, expected, 1e-9);
    }

    // Edge case: window size 1
    SortedWindowMedian k_small;
    k_small.configure(1);
    rtk::near(run, rtk::guarded(run, [&] { return k_small.push(10.0); }), 10.0);
    rtk::near(run, rtk::guarded(run, [&] { return k_small.push(20.0); }), 20.0);

    // Edge case: window size 0
    SortedWindowMedian k_zero;
    k_zero.configure(0);
    rtk::near(run, rtk::guarded(run, [&] { return k_zero.push(10.0); }), 0.0);

    return rtk::report(run);
}
