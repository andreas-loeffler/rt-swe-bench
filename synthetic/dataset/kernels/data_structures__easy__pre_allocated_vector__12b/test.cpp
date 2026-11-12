#include "kernel.hpp"
#include "rtk/check.hpp"
#include <vector>

int main() {
    rtk::setup();
    rtk::Run run;

    const size_t capacity = 100;
    PreAllocatedVector<int> vec;
    vec.configure(capacity);

    // Test 1: Fill to capacity
    for (int i = 0; i < (int)capacity; ++i) {
        bool success = rtk::guarded(run, [&] { return vec.push_back(i); });
        rtk::expect(run, success == true, "Push successful within capacity");
    }

    // Test 2: Push at capacity (Overflow)
    bool overflow = rtk::guarded(run, [&] { return vec.push_back(999); });
    rtk::expect(run, overflow == false, "Push failed at capacity");

    // Test 3: Access back
    int last_val = rtk::guarded(run, [&] { return vec.back(); });
    rtk::expect(run, last_val == (int)capacity - 1, "Correct back value");

    // Test 4: Stress test (Repeated clear/push pattern)
    // Since we don't have a clear() method, we'll simulate heavy usage 
    // by creating multiple instances or checking behavior repeatedly.
    // However, the prompt asks for 500 calls. Let's use a loop.
    
    for (int i = 0; i < 500; ++i) {
        // Create a fresh instance for each iteration to reset state
        PreAllocatedVector<int> local_vec;
        local_vec.configure(10);
        
        // Perform a deterministic sequence of operations
        bool res1 = rtk::guarded(run, [&] { return local_vec.push_back(i % 10); });
        rtk::expect(run, res1 == true, "Batch 1");
        
        bool res2 = rtk::guarded(run, [&] { return local_vec.push_back(i % 10); });
        rtk::expect(run, res2 == true, "Batch 2");
        
        int b = rtk::guarded(run, [&] { return local_vec.back(); });
        // The last value added is (i%10), but there's another one before it.
        // Since we push twice, and local_vec was empty:
        // Index 0: i%10
        // Index 1: i%10
        // Wait, the logic above pushes same value twice.
        rtk::expect(run, b == (i % 10), "Batch back check");
    }

    return rtk::report(run);
}
