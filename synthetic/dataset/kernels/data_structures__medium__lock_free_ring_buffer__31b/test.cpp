#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <string>

int main() {
    rtk::setup();
    rtk::Run run;

    LockFreeRingBuffer<int> k;
    const size_t capacity = 10;
    k.configure(capacity);

    // Test 1: Empty state
    int val;
    rtk::expect(run, rtk::guarded(run, [&] { return k.pop(val); }) == false, "Initial pop fails");

    // Test 2: Fill to capacity
    for (int i = 0; i < (int)capacity; ++i) {
        rtk::expect(run, rtk::guarded(run, [&] { return k.push(i); }), "Push " + std::to_string(i));
    }

    // Test 3: Overfill
    rtk::expect(run, rtk::guarded(run, [&] { return k.push(99); }) == false, "Buffer is full");

    // Test 4: Drain
    for (int i = 0; i < (int)capacity; ++i) {
        rtk::guarded_void(run, [&] {
            bool success = k.pop(val);
            rtk::expect(run, success && val == i, "Pop expected " + std::to_string(i));
        });
    }

    // Test 5: Empty again
    rtk::expect(run, rtk::guarded(run, [&] { return k.pop(val); }) == false, "Buffer empty again");

    // Test 6: Stress/Wrap-around (1000 calls)
    const int iterations = 1000;
    for (int i = 0; i < iterations; ++i) {
        rtk::guarded_void(run, [&] {
            bool pushed = k.push(i);
            rtk::expect(run, pushed, "Stress push " + std::to_string(i));
            
            bool popped = k.pop(val);
            rtk::expect(run, popped && val == i, "Stress pop " + std::to_string(i));
        });
    }

    // Test 7: Partial fill and wrap
    for (int i = 0; i < 5; ++i) rtk::guarded_void(run, [&] { k.push(i); });
    for (int i = 0; i < 5; ++i) rtk::guarded_void(run, [&] { 
        bool ok = k.pop(val);
        rtk::expect(run, ok && val == i, "Partial pop " + std::to_string(i));
    });
    
    // Fill to capacity
    for (int i = 0; i < (int)capacity; ++i) rtk::guarded_void(run, [&] { k.push(i + 100); });
    
    rtk::expect(run, rtk::guarded(run, [&] { return k.push(999); }) == false, "Wrapped buffer full");
    
    for (int i = 0; i < (int)capacity; ++i) {
        rtk::guarded_void(run, [&] {
            bool ok = k.pop(val);
            rtk::expect(run, ok && val == (i + 100), "Wrapped pop " + std::to_string(i + 100));
        });
    }

    return rtk::report(run);
}
