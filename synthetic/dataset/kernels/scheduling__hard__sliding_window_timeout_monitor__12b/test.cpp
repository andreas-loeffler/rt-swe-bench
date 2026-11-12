#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    // Configuration: Window of 10, Threshold of 5.0ms
    TimeoutMonitor k;
    k.configure(10, 5.0);

    // Test Case 1: Empty/Small window
    rtk::guarded(run, [&] { return k.push(100.0); });
    rtk::expect(run, true, "Push first element");
    rtk::expect(run, k.check_timeout() == false, "Check timeout empty/single");

    // Test Case 2: Normal arrivals (Average 2.0ms < 5.0ms)
    // 100.0, 102.0, 104.0, 106.0, 108.0, 110.0, 112.0, 114.0, 116.0, 118.0
    for (int i = 1; i <= 9; ++i) {
        double ts = 100.0 + (i * 2.0);
        rtk::guarded(run, [&] { return k.push(ts); });
        if (i < 9) {
            rtk::expect(run, k.check_timeout() == false, "Check timeout under threshold");
        }
    }
    // After 10 elements, average gap is 2.0. 
    rtk::expect(run, k.check_timeout() == false, "Check timeout 10 elements average 2.0");

    // Test Case 3: Sudden spike (Average > 5.0ms)
    // Last gap was 2.0. Next gap is 100.0. 
    // Sum of gaps: (9 * 2.0) + 100.0 = 118.0. 
    // Number of gaps = 9. Avg = 118.0 / 9 = 13.11111111 > 5.0.
    rtk::guarded(run, [&] { return k.push(118.0 + 100.0); });
    rtk::expect(run, k.check_timeout() == true, "Check timeout over threshold");

    // Test Case 4: Wrap around and sliding behavior
    // We have 10 elements. Pushing one more replaces the oldest (100.0).
    // The window now contains: 102, 104, 106, 108, 110, 112, 114, 116, 118, 218.
    // Gaps: 2,2,2,2,2,2,2,2,100. Sum = 118. Avg = 118/9 = 13.11. Still true.
    
    // Push more small gaps to bring average down.
    // Current sum of gaps = 118. We need to remove the 100 gap and add small ones.
    // To remove 100, we need to push 10 more elements.
    for (int i = 1; i <= 10; ++i) {
        double ts = 218.0 + (i * 1.0);
        rtk::guarded(run, [&] { return k.push(ts); });
    }
    // The 100 gap is gone. New gaps are all 1.0.
    rtk::expect(run, k.check_timeout() == false, "Check timeout average lowered");

    // Test Case 5: Edge case - very small window
    TimeoutMonitor k2;
    k2.configure(2, 1.0);
    rtk::guarded(run, [&] { return k2.push(100.0); });
    rtk::guarded(run, [&] { return k2.push(102.0); });
    rtk::expect(run, k2.check_timeout() == true, "Check timeout small window"); // Gap 2.0 > 1.0

    // Test Case 6: Verify threshold exactness
    TimeoutMonitor k3;
    k3.configure(2, 2.0);
    rtk::guarded(run, [&] { return k3.push(100.0); });
    rtk::guarded(run, [&] { return k3.push(102.0); });
    // Average gap is 2.0. Threshold is 2.0. 2.0 > 2.0 is false.
    rtk::expect(run, k3.check_timeout() == false, "Check timeout exact threshold (2.0 > 2.0 is false)");

    // Test Case 7: Verify non-monotonic behavior (though not required by API, checks robustness)
    // Note: The spec says assume monotonic, but we check that it doesn't crash or give wild results.
    TimeoutMonitor k4;
    k4.configure(3, 5.0);
    rtk::guarded(run, [&] { return k4.push(100.0); });
    rtk::guarded(run, [&] { return k4.push(150.0); }); // Gap 50
    rtk::guarded(run, [&] { return k4.push(140.0); }); // Gap -10
    // Sum of gaps: 50 + (-10) = 40. Avg = 40 / 2 = 20.0 > 5.0.
    rtk::expect(run, k4.check_timeout() == true, "Check timeout with non-monotonic input");

    return rtk::report(run);
}
