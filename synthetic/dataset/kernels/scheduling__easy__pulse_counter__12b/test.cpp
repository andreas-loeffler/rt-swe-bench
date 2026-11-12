#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    const size_t threshold = 100;
    PulseCounter k;
    k.configure(threshold);

    // State variables for test
    bool prev = false;
    size_t expected_count = 0;

    // Test 1: No edges
    for (int i = 0; i < 100; ++i) {
        bool curr = false;
        bool result = rtk::guarded(run, [&] { return k.process(curr, prev); });
        rtk::expect(run, result == false, "No edge should return false");
        prev = curr;
    }

    // Test 2: Rising edges within threshold
    for (int i = 0; i < 150; ++i) {
        bool curr = (i % 2 == 0); // Create pulses
        bool result = rtk::guarded(run, [&] { return k.process(curr, prev); });
        
        if (curr && !prev) {
            expected_count++;
            bool expected_res = (expected_count <= threshold);
            rtk::expect(run, result == expected_res, "Rising edge " + std::to_string(expected_count));
        }
        prev = curr;
    }

    // Test 3: Falling edges
    for (int i = 0; i < 100; ++i) {
        bool curr = false;
        bool result = rtk::guarded(run, [&] { return k.process(curr, prev); });
        rtk::expect(run, result == false, "Falling edge should return false");
        prev = curr;
    }

    // Test 4: Exhaust threshold
    // We already did 150 iterations, some were rising edges. 
    // Let's just do many more to ensure it stays false after threshold.
    for (int i = 0; i < 500; ++i) {
        bool curr = true;
        bool prev_val = false;
        // Force a rising edge
        bool result = rtk::guarded(run, [&] { return k.process(curr, prev_val); });
        
        expected_count++;
        bool expected_res = (expected_count <= threshold);
        rtk::expect(run, result == expected_res, "Exhausting threshold at " + std::to_string(expected_count));
        
        prev_val = curr; // stay high
        k.process(prev_val, prev_val); // ensure it doesn't trigger again
    }

    return rtk::report(run);
}
