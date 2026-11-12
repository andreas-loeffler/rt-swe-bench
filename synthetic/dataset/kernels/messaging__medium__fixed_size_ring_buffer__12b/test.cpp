#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>
#include <numeric>

int main() {
    rtk::setup();
    rtk::Run run;

    const size_t capacity = 10;
    FixedRingBuffer<int> k;
    k.configure(capacity);

    // Test 1: Fill the buffer
    for (int i = 0; i < 10; ++i) {
        bool success = rtk::guarded(run, [&] { return k.push(i); });
        rtk::expect(run, success == true, "Push 0-9");
    }

    // Test 2: Overflow
    bool overflow = rtk::guarded(run, [&] { return k.push(100); });
    rtk::expect(run, overflow == false, "Push 11th element (overflow)");

    // Test 3: Pop all
    std::vector<int> expected_pops;
    for (int i = 0; i < 10; ++i) {
        int val;
        bool success = rtk::guarded(run, [&] { return k.pop(val); });
        rtk::expect(run, success == true, "Pop index " + std::to_string(i));
        expected_pops.push_back(val);
    }

    for (size_t i = 0; i < expected_pops.size(); ++i) {
        rtk::expect(run, expected_pops[i] == (int)i, "Correct order");
    }

    // Test 4: Empty pop
    int dummy;
    bool empty_pop = rtk::guarded(run, [&] { return k.pop(dummy); });
    rtk::expect(run, empty_pop == false, "Pop from empty");

    // Test 5: Wrap-around check
    // Fill 5, Pop 5, Fill 5 (wrap-around)
    for (int i = 0; i < 5; ++i) rtk::guarded_void(run, [&] { k.push(i); });
    for (int i = 0; i < 5; ++i) {
        int v;
        rtk::guarded_void(run, [&] { k.pop(v); });
    }
    for (int i = 100; i < 105; ++i) {
        bool success = rtk::guarded(run, [&] { return k.push(i); });
        rtk::expect(run, success == true, "Wrap-around push");
    }

    // Test 6: Stress wrap-around
    // Repeatedly push/pop to ensure pointers cycle many times
    for (int i = 0; i < 500; ++i) {
        int val = i % 1000;
        bool p = rtk::guarded(run, [&] { return k.push(val); });
        if (p) {
            int out;
            bool o = rtk::guarded(run, [&] { return k.pop(out); });
            rtk::expect(run, o == true, "Sequential stress");
        }
    }

    return rtk::report(run);
}
