#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    PeakDetector k;
    k.configure(10.0f, 3);

    // Test 1: Basic peak detection
    // Sequence: 0, 5, 15, 10, 0
    // Index:    0, 1,  2,  3,  4
    // Peak at 2 confirmed at index 3
    std::vector<float> data1 = {0.0f, 5.0f, 15.0f, 10.0f, 0.0f};
    bool last_res = false;
    for (float val : data1) {
        last_res = rtk::guarded(run, [&] { return k.process(val); });
    }
    rtk::expect(run, last_res == false, "Final result should be false");
    // The peak was confirmed when 10.0f was processed.
    // Let's re-run to catch the specific true.
    
    PeakDetector k2;
    k2.configure(10.0f, 3);
    last_res = false;
    bool caught_peak = false;
    for (float val : data1) {
        last_res = rtk::guarded(run, [&] { return k2.process(val); });
        if (last_res) caught_peak = true;
    }
    rtk::expect(run, caught_peak == true, "Should detect peak at index 2");

    // Test 2: Distance constraint
    // Peaks at index 2 and 4 are too close (dist=2, required=3)
    // Sequence: 0, 20, 0, 20, 0
    std::vector<float> data2 = {0.0f, 20.0f, 0.0f, 20.0f, 0.0f};
    PeakDetector k3;
    k3.configure(10.0f, 3);
    int peak_count = 0;
    for (float val : data2) {
        if (rtk::guarded(run, [&] { return k3.process(val); })) {
            peak_count++;
        }
    }
    rtk::expect(run, peak_count == 1, "Should only detect first peak due to distance");

    // Test 3: Threshold check
    // Peak is 9.0, threshold is 10.0
    std::vector<float> data3 = {0.0f, 9.0f, 0.0f};
    PeakDetector k4;
    k4.configure(10.0f, 1);
    bool caught_low_peak = false;
    for (float val : data3) {
        if (rtk::guarded(run, [&] { return k4.process(val); })) {
            caught_low_peak = true;
        }
    }
    rtk::expect(run, caught_low_peak == false, "Should not detect peak below threshold");

    // Test 4: Long stream with many peaks
    // Generate a zigzag: 0, 50, 0, 50, 0, 50, 0...
    // Distance 2, threshold 10.
    PeakDetector k5;
    k5.configure(10.0f, 2);
    int total_peaks = 0;
    for (int i = 0; i < 100; ++i) {
        float val = (i % 2 == 0) ? 0.0f : 50.0f;
        // This produces peaks every 4 samples (0, 50, 0, 50, 0 -> peak at index 1 and 3)
        // But distance is 2. Index 1 is peak. Index 3 is peak (dist=2). 
        // Wait, index 1 is peak, confirmed at i=2. Next peak is index 3, confirmed at i=4.
        // Distance 4-2 = 2. So both should be caught.
        if (rtk::guarded(run, [&] { return k5.process(val); })) {
            total_peaks++;
        }
    }
    // In 100 samples, we have 50 '50's. 
    // Each '50' at index j is a peak if j is odd. 
    // Indices: 1, 3, 5, 7...
    // Peaks confirmed at: 2, 4, 6, 8...
    // Distance between confirmations: 2. 
    // Requirement: distance >= 2.
    // So every peak should be caught.
    // Total peaks in 100 samples: 100/2 = 50, but we need 3 points for first peak.
    // First peak is at index 1, confirmed at index 2.
    // Last sample is index 99. Last peak is at index 97, confirmed at index 98.
    // Total peaks: 1, 3, 5 ... 97 -> (97-1)/2 + 1 = 49.
    rtk::expect(run, total_peaks == 49, "Should detect 49 peaks in 100 samples");

    return rtk::report(run);
}
