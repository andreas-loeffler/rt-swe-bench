#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    RingBufferLogger logger;
    const size_t capacity = 100;
    logger.configure(capacity);

    std::vector<float> expected;
    
    // Test 1: Filling the buffer (0 to 100)
    for (int i = 0; i < 1000; ++i) {
        float val = static_cast<float>(i);
        
        rtk::guarded_void(run, [&] { logger.push(val); });
        
        // Track expected values in a standard vector (allowed outside guarded calls)
        expected.push_back(val);
        if (expected.size() > capacity) {
            expected.erase(expected.begin());
        }

        size_t s = rtk::guarded(run, [&] { return logger.size(); });
        rtk::expect(run, s == expected.size(), "size_check_" + std::to_string(i));

        // Sample check for the most recent element
        if (s > 0) {
            float last = rtk::guarded(run, [&] { return logger.get(s - 1); });
            rtk::near(run, last, expected.back());
        }
    }

    // Test 2: Detailed verification of wrap-around and indexing
    // At this point, expected contains values [900, 999]
    for (size_t i = 0; i < expected.size(); ++i) {
        float val = rtk::guarded(run, [&] { return logger.get(i); });
        rtk::near(run, val, expected[i]);
    }

    // Test 3: Push 10 more and verify window shifted
    for (int i = 1000; i < 1010; ++i) {
        float val = static_cast<float>(i);
        rtk::guarded_void(run, [&] { logger.push(val); });
        expected.push_back(val);
        expected.erase(expected.begin());
    }

    for (size_t i = 0; i < expected.size(); ++i) {
        float val = rtk::guarded(run, [&] { return logger.get(i); });
        rtk::near(run, val, expected[i]);
    }

    return rtk::report(run);
}
